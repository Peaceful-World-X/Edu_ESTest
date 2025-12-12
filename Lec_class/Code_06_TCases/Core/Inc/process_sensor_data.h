#ifndef __PROCESS_SENSOR_DATA_H
#define __PROCESS_SENSOR_DATA_H

#include <stdint.h>

#define TOTAL_STATEMENTS 84
#define TOTAL_BRANCHES 69

// 代码覆盖率统计变量（外部声明）
extern uint32_t branch_coverage[];
extern uint32_t statement_coverage[];
extern uint32_t executed_statements;
extern uint32_t executed_branches;

/**
 * @brief 传感器数据处理与告警系统（多分支结构，适合代码覆盖率测试）
 *
 * @param mode      工作模式 (1=正常监测, 2=节能模式, 3=高精度模式, 4=校准模式)
 * @param temp      温度值 (-50 到 150，单位：℃)
 * @param humidity  湿度值 (0 到 100，单位：%)
 * @param light     光强值 (0 到 65535，单位：lx)
 * @param key       键盘按键值 (0-16)
 * @return 返回处理结果代码
 *
 * 返回值说明：
 *  100+ : 正常状态码
 *  200+ : 警告状态码
 *  300+ : 错误状态码
 *  400+ : 严重告警状态码
 */
int process_sensor_data(int mode, int temp, int humidity, int light, int key);

#endif /* __PROCESS_SENSOR_DATA_H */

