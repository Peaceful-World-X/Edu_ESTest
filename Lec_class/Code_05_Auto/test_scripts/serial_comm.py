"""
    提供基础串口通信功能框架
"""
import serial
import time
import matplotlib.pyplot as plt


class SerialDevice:
    """串口设备类"""

    def __init__(self, port: str = 'COM5', baudrate: int = 115200):
        """初始化串口参数"""
        self.port = port
        self.baudrate = baudrate
        self.ser = None

    def connect(self):
        """连接串口"""
        try:
            self.ser = serial.Serial(self.port, self.baudrate, timeout=1.0)
            time.sleep(0.5)
            return True
        except Exception as e:
            print(f"✗ 连接失败: {e}")
            return False

    def disconnect(self):
        """断开串口"""
        if self.ser and self.ser.is_open:
            self.ser.close()

    def send(self, command: str) -> bool:
        """发送命令"""
        if not self.ser or not self.ser.is_open:
            return False
        try:
            self.ser.write((command + '\r\n').encode())
            return True
        except Exception as e:
            print(f"✗ 发送失败: {e}")
            return False

    def read(self) -> str:
        """读取响应"""
        if not self.ser or not self.ser.is_open:
            return ""
        try:
            return self.ser.read_all().decode('utf-8', errors='ignore')
        except Exception as e:
            print(f"✗ 读取失败: {e}")
            return ""

    def test_command(self, mode: int, temp: int, humidity: int, time_sec: int) -> str:
        """
        发送TEST命令并返回响应
        格式: TEST <mode> <temp> <humidity> <time>
        """
        cmd = f"TEST {mode} {temp} {humidity} {time_sec}"
        if self.send(cmd):
            time.sleep(0.2)
            return self.read()
        return ""
