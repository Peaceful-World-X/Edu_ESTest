"""
pytest测试模板
演示基础测试框架和数据可视化
"""
import pytest
import time
import matplotlib.pyplot as plt
import numpy as np
from serial_comm import SerialDevice

SERIAL_PORT = 'COM5'

# ========== Fixtures ==========
@pytest.fixture(scope="module")
def device():
    """创建并连接串口设备"""
    dev = SerialDevice(port=SERIAL_PORT, baudrate=9600)
    if not dev.connect():
        print(f"❌ 串口连接失败!")
    yield dev
    dev.disconnect()


# ========== 测试类示例 ==========
class TestBasicFunctionality:
    """基础功能测试类 - 模板"""

    def test_valid_input(self, device):
        """测试有效输入"""
        response = device.test_command(mode=1, temp=25, humidity=50, time_sec=43200)
        assert response, "应该收到响应"

    @pytest.mark.parametrize("temp,expected", [
        (-51, "error"),   # 低于最小值
        (151, "error"),   # 超过最大值
    ])
    def test_temp_boundaries(self, device, temp, expected):
        """参数化测试：温度边界值"""
        response = device.test_command(mode=1, temp=temp, humidity=50, time_sec=43200)
        if expected == "error":
            assert "301" in response or "Error" in response
        else:
            assert response
