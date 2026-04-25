#!/usr/bin/env python3
"""Simple UART monitor for STM32 controller output.

Usage examples:
  python test/serial_monitor.py --port COM3
  python test/serial_monitor.py --port COM3 --baud 9600 --raw
"""

import argparse
import os
import queue
import re
import socket
import subprocess
import sys
import threading
import time
from collections import deque
from typing import Optional

try:
    import serial
    from serial import SerialException
except ImportError:
    print("Missing dependency: pyserial")
    print("Install with: pip install pyserial")
    sys.exit(1)

try:
    import matplotlib.pyplot as plt
except ImportError:
    plt = None


CPU_PATTERN = re.compile(r"CPU:([0-9]+(?:\.[0-9]+)?)%")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Realtime UART receiver")
    parser.add_argument("--port", default="COM4", help="Serial port, e.g. COM3 or /dev/ttyUSB0")
    parser.add_argument("--baud", type=int, default=9600, help="Baud rate (default: 9600)")
    parser.add_argument(
        "--timeout",
        type=float,
        default=0.2,
        help="Read timeout in seconds (default: 0.2)",
    )
    parser.add_argument(
        "--raw",
        action="store_true",
        help="Print every received chunk without line buffering",
    )
    parser.add_argument(
        "--no-plot-cpu",
        action="store_true",
        help="Disable realtime CPU waveform plotting",
    )
    parser.add_argument(
        "--window",
        type=int,
        default=120,
        help="Number of points shown in CPU plot window (default: 120)",
    )
    parser.add_argument(
        "--tx-console",
        action="store_true",
        help="Open a second console window for sending UART messages",
    )
    parser.add_argument(
        "--tx-host",
        default="127.0.0.1",
        help="Host for local TX bridge (default: 127.0.0.1)",
    )
    parser.add_argument(
        "--tx-port",
        type=int,
        default=8765,
        help="Port for local TX bridge (default: 8765)",
    )
    parser.add_argument(
        "--tx-client",
        action="store_true",
        help=argparse.SUPPRESS,
    )
    return parser.parse_args()


def open_serial(port: str, baud: int, timeout: float) -> serial.Serial:
    return serial.Serial(
        port=port,
        baudrate=baud,
        bytesize=serial.EIGHTBITS,
        parity=serial.PARITY_NONE,
        stopbits=serial.STOPBITS_ONE,
        timeout=timeout,
    )


def flush_tx_queue(ser: serial.Serial, tx_queue: Optional[queue.Queue[str]]) -> None:
    if tx_queue is None:
        return

    while True:
        try:
            message = tx_queue.get_nowait()
        except queue.Empty:
            break

        payload = (message + "\r\n").encode("utf-8", errors="replace")
        ser.write(payload)
        ts = time.strftime("%H:%M:%S")
        print(f"[{ts}] TX> {message}")


def monitor_lines(ser: serial.Serial, tx_queue: Optional[queue.Queue[str]] = None) -> None:
    while True:
        flush_tx_queue(ser, tx_queue)
        line = ser.readline()
        if not line:
            continue
        text = line.decode("utf-8", errors="replace").rstrip("\r\n")
        ts = time.strftime("%H:%M:%S")
        print(f"[{ts}] {text}")


def extract_cpu(text: str) -> float | None:
    match = CPU_PATTERN.search(text)
    if match is None:
        return None
    try:
        return float(match.group(1))
    except ValueError:
        return None


def monitor_lines_with_cpu_plot(
    ser: serial.Serial,
    window: int,
    tx_queue: Optional[queue.Queue[str]] = None,
) -> None:
    if plt is None:
        print("Missing dependency: matplotlib")
        print("Install with: pip install matplotlib")
        print("Fallback to line monitor mode.")
        monitor_lines(ser, tx_queue)
        return

    x_values: deque[int] = deque(maxlen=max(10, window))
    cpu_values: deque[float] = deque(maxlen=max(10, window))
    sample_index = 0

    plt.ion()
    fig, ax = plt.subplots(num="CPU Usage Realtime")
    line_plot, = ax.plot([], [], color="#1f77b4", linewidth=2)
    ax.set_title("Controller CPU Usage")
    ax.set_xlabel("Samples")
    ax.set_ylabel("CPU %")
    ax.set_ylim(0, 100)
    ax.grid(True, linestyle="--", alpha=0.4)
    fig.tight_layout()

    while plt.fignum_exists(fig.number):
        flush_tx_queue(ser, tx_queue)
        line = ser.readline()
        if line:
            text = line.decode("utf-8", errors="replace").rstrip("\r\n")
            ts = time.strftime("%H:%M:%S")
            print(f"[{ts}] {text}")

            cpu = extract_cpu(text)
            if cpu is not None:
                sample_index += 1
                x_values.append(sample_index)
                cpu_values.append(cpu)
                line_plot.set_data(list(x_values), list(cpu_values))

                if sample_index < window:
                    ax.set_xlim(1, window)
                else:
                    ax.set_xlim(sample_index - window + 1, sample_index)

                fig.canvas.draw_idle()

        plt.pause(0.01)

    print("Plot window closed. Stop monitor.")


def monitor_raw(ser: serial.Serial) -> None:
    while True:
        chunk = ser.read(ser.in_waiting or 1)
        if not chunk:
            continue
        text = chunk.decode("utf-8", errors="replace")
        print(text, end="", flush=True)


def tx_server_worker(
    host: str,
    port: int,
    tx_queue: queue.Queue[str],
    stop_event: threading.Event,
) -> None:
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server:
        server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        server.bind((host, port))
        server.listen(1)
        server.settimeout(0.5)

        while not stop_event.is_set():
            try:
                conn, _ = server.accept()
            except socket.timeout:
                continue
            except OSError:
                break

            with conn:
                conn.settimeout(0.5)
                buffer = ""
                while not stop_event.is_set():
                    try:
                        data = conn.recv(1024)
                    except socket.timeout:
                        continue
                    except OSError:
                        break

                    if not data:
                        break

                    buffer += data.decode("utf-8", errors="replace")
                    while "\n" in buffer:
                        line, buffer = buffer.split("\n", 1)
                        line = line.rstrip("\r")
                        if line:
                            tx_queue.put(line)


def tx_client_console(host: str, port: int) -> int:
    try:
        with socket.create_connection((host, port), timeout=5) as conn:
            print(f"TX console connected to {host}:{port}")
            print("Input text and press Enter to send. Type exit to quit.")
            while True:
                try:
                    message = input("TX> ").strip()
                except EOFError:
                    break

                if not message:
                    continue
                if message.lower() in {"exit", "quit"}:
                    break

                conn.sendall((message + "\n").encode("utf-8", errors="replace"))
    except OSError as exc:
        print(f"TX client error: {exc}")
        return 4

    return 0


def start_tx_console(script_path: str, host: str, port: int) -> None:
    child_cmd = (
        f'python "{script_path}" --tx-client --tx-host {host} --tx-port {port}'
    )
    ps_command = (
        "Start-Process powershell "
        f"-ArgumentList @('-NoExit','-Command','{child_cmd}')"
    )
    subprocess.Popen(
        ["powershell", "-NoProfile", "-Command", ps_command],
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    )


def main() -> int:
    args = parse_args()

    if args.tx_client:
        return tx_client_console(args.tx_host, args.tx_port)

    print(f"Opening {args.port} @ {args.baud} ...")

    tx_queue: Optional[queue.Queue[str]] = None
    stop_event = threading.Event()
    tx_thread: Optional[threading.Thread] = None

    if args.tx_console:
        tx_queue = queue.Queue()
        tx_thread = threading.Thread(
            target=tx_server_worker,
            args=(args.tx_host, args.tx_port, tx_queue, stop_event),
            daemon=True,
        )
        tx_thread.start()
        script_path = os.path.abspath(__file__)
        time.sleep(0.15)
        start_tx_console(script_path, args.tx_host, args.tx_port)
        print(f"TX bridge listening at {args.tx_host}:{args.tx_port}")

    try:
        with open_serial(args.port, args.baud, args.timeout) as ser:
            print("Connected. Press Ctrl+C to stop.")
            if args.raw:
                monitor_raw(ser)
            elif args.no_plot_cpu:
                monitor_lines(ser, tx_queue)
            else:
                monitor_lines_with_cpu_plot(ser, args.window, tx_queue)
    except KeyboardInterrupt:
        print("\nStopped by user.")
        return 0
    except SerialException as exc:
        print(f"Serial error: {exc}")
        return 2
    except OSError as exc:
        print(f"OS error: {exc}")
        return 3
    finally:
        stop_event.set()
        if tx_thread is not None:
            tx_thread.join(timeout=0.5)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
