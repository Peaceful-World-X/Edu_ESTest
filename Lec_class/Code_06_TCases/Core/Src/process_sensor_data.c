#include "process_sensor_data.h"

uint32_t branch_coverage[100] = {0};        // 记录每个分支的执行次数
uint32_t statement_coverage[100] = {0};     // 记录每个语句的执行次数
uint32_t executed_statements = 0;           // 本次执行已执行语句数
uint32_t executed_branches = 0;             // 本次执行已执行分支数

int process_sensor_data(int mode, int temp, int humidity, int light, int key) {
    int result_code = 0;
    int is_daytime = 0;
    int temp_level = 0;
    int humidity_level = 0;
    int light_level = 0;
    int key_mode = 0;
    executed_statements = 0;
    executed_branches = 0;
    statement_coverage[0]++; executed_statements++;
    statement_coverage[1]++; executed_statements++;
    if (temp < -50 || temp > 150) {
        branch_coverage[0]++; executed_branches++;
        statement_coverage[2]++; executed_statements++;
        return 301;
    }
    branch_coverage[1]++; executed_branches++;

    statement_coverage[3]++; executed_statements++;
    if (humidity < 0 || humidity > 100) {
        branch_coverage[2]++; executed_branches++;
        statement_coverage[4]++; executed_statements++;
        return 302;
    }
    branch_coverage[3]++; executed_branches++;

    statement_coverage[5]++; executed_statements++;
    if (light < 0 || light > 65535) {
        branch_coverage[4]++; executed_branches++;
        statement_coverage[6]++; executed_statements++;
        return 305;  // 光强参数错误
    }
    branch_coverage[5]++; executed_branches++;

    statement_coverage[7]++; executed_statements++;
    if (key < 0 || key > 16) {
        branch_coverage[6]++; executed_branches++;
        statement_coverage[8]++; executed_statements++;
        return 306;  // 键盘参数错误
    }
    branch_coverage[7]++; executed_branches++;
    statement_coverage[9]++; executed_statements++;
    if (light >= 1000) {
        branch_coverage[8]++; executed_branches++;
        is_daytime = 1;
        statement_coverage[10]++; executed_statements++;
    } else {
        branch_coverage[9]++; executed_branches++;
        is_daytime = 0;
        statement_coverage[11]++; executed_statements++;
    }
    statement_coverage[12]++; executed_statements++;
    if (light < 100) {
        branch_coverage[10]++; executed_branches++;
        light_level = 0;  // 很暗
        statement_coverage[13]++; executed_statements++;
    } else if (light < 1000) {
        branch_coverage[11]++; executed_branches++;
        light_level = 1;  // 较暗
        statement_coverage[14]++; executed_statements++;
    } else if (light < 10000) {
        branch_coverage[12]++; executed_branches++;
        light_level = 2;  // 正常
        statement_coverage[15]++; executed_statements++;
    } else {
        branch_coverage[13]++; executed_branches++;
        light_level = 3;  // 很亮
        statement_coverage[16]++; executed_statements++;
    }
    statement_coverage[17]++; executed_statements++;
    if (key > 0 && key <= 4) {
        branch_coverage[14]++; executed_branches++;
        key_mode = key;
        statement_coverage[18]++; executed_statements++;
    } else {
        branch_coverage[15]++; executed_branches++;
        key_mode = mode;  // 使用传入的模式
        statement_coverage[19]++; executed_statements++;
    }
    statement_coverage[20]++; executed_statements++;
    if (temp < -10) {
        branch_coverage[16]++; executed_branches++;
        temp_level = 3;
        statement_coverage[21]++; executed_statements++;
    } else if (temp < 10) {
        branch_coverage[17]++; executed_branches++;
        temp_level = 1;
        statement_coverage[22]++; executed_statements++;
    } else if (temp <= 35) {
        branch_coverage[18]++; executed_branches++;
        temp_level = 0;
        statement_coverage[23]++; executed_statements++;
    } else if (temp <= 50) {
        branch_coverage[19]++; executed_branches++;
        temp_level = 2;
        statement_coverage[24]++; executed_statements++;
    } else {
        branch_coverage[20]++; executed_branches++;
        temp_level = 3;
        statement_coverage[25]++; executed_statements++;
    }
    statement_coverage[26]++; executed_statements++;
    if (humidity < 30) {
        branch_coverage[21]++; executed_branches++;
        humidity_level = 1;
        statement_coverage[27]++; executed_statements++;
    } else if (humidity <= 70) {
        branch_coverage[22]++; executed_branches++;
        humidity_level = 0;
        statement_coverage[28]++; executed_statements++;
    } else {
        branch_coverage[23]++; executed_branches++;
        humidity_level = 2;
        statement_coverage[29]++; executed_statements++;
    }
    statement_coverage[30]++; executed_statements++;
    switch (key_mode) {
        case 1:
            branch_coverage[24]++; executed_branches++;
            result_code = 100;
            statement_coverage[31]++; executed_statements++;

            statement_coverage[32]++; executed_statements++;
            if (temp_level == 3 || (temp_level == 2 && humidity > 50)) {
                branch_coverage[25]++; executed_branches++;
                statement_coverage[33]++; executed_statements++;
                if (temp_level == 3) {
                    branch_coverage[26]++; executed_branches++;
                    result_code = 401;
                    statement_coverage[34]++; executed_statements++;
                } else {
                    branch_coverage[27]++; executed_branches++;
                    result_code = 201;
                    statement_coverage[35]++; executed_statements++;
                }
            } else {
                branch_coverage[28]++; executed_branches++;
                statement_coverage[36]++; executed_statements++;
            }
            statement_coverage[37]++; executed_statements++;
            if (temp_level == 1 && (humidity < 30 || humidity > 70)) {
                branch_coverage[29]++; executed_branches++;
                statement_coverage[38]++; executed_statements++;
                if (humidity < 30) {
                    branch_coverage[30]++; executed_branches++;
                    result_code = 202;
                    statement_coverage[39]++; executed_statements++;
                } else {
                    branch_coverage[31]++; executed_branches++;
                    result_code = 204;
                    statement_coverage[40]++; executed_statements++;
                }
            } else {
                branch_coverage[32]++; executed_branches++;
                statement_coverage[41]++; executed_statements++;
            }
            statement_coverage[42]++; executed_statements++;
            if (temp_level == 0 && humidity_level == 2 && light_level >= 2) {
                branch_coverage[33]++; executed_branches++;
                result_code = 203;
                statement_coverage[43]++; executed_statements++;
            } else {
                branch_coverage[34]++; executed_branches++;
                statement_coverage[44]++; executed_statements++;
            }
            statement_coverage[45]++; executed_statements++;
            if (temp_level >= 2 && humidity_level == 2 && (light_level == 0 || light_level == 3)) {
                branch_coverage[35]++; executed_branches++;
                result_code = 402;
                statement_coverage[46]++; executed_statements++;
            } else {
                branch_coverage[36]++; executed_branches++;
                statement_coverage[47]++; executed_statements++;
            }
            break;

        case 2:
            branch_coverage[37]++; executed_branches++;
            result_code = 110;
            statement_coverage[48]++; executed_statements++;
            statement_coverage[49]++; executed_statements++;
            if (is_daytime == 0 && (temp_level == 0 || humidity < 40)) {
                branch_coverage[38]++; executed_branches++;
                statement_coverage[50]++; executed_statements++;
                if (temp_level == 0) {
                    branch_coverage[39]++; executed_branches++;
                    result_code = 111;
                    statement_coverage[51]++; executed_statements++;
                } else {
                    branch_coverage[40]++; executed_branches++;
                    result_code = 211;
                    statement_coverage[52]++; executed_statements++;
                }
            } else {
                branch_coverage[41]++; executed_branches++;
                result_code = 212;
                statement_coverage[53]++; executed_statements++;
            }
            break;

        case 3:
            branch_coverage[42]++; executed_branches++;
            result_code = 120;
            statement_coverage[54]++; executed_statements++;
            statement_coverage[55]++; executed_statements++;
            if ((temp >= 15 && temp <= 30) && (humidity >= 40 && humidity <= 60) && (light >= 500 && light <= 5000)) {
                branch_coverage[43]++; executed_branches++;
                result_code = 121;
                statement_coverage[56]++; executed_statements++;
            } else {
                branch_coverage[44]++; executed_branches++;
                statement_coverage[57]++; executed_statements++;
                if (temp >= 15 && temp <= 30) {
                    branch_coverage[45]++; executed_branches++;
                    statement_coverage[58]++; executed_statements++;
                    if (humidity >= 40 && humidity <= 60) {
                        branch_coverage[46]++; executed_branches++;
                        result_code = 221;  // 温度湿度OK但光强不在范围
                        statement_coverage[59]++; executed_statements++;
                    } else {
                        branch_coverage[47]++; executed_branches++;
                        result_code = 222;  // 温度OK但湿度不在范围
                        statement_coverage[60]++; executed_statements++;
                    }
                } else {
                    branch_coverage[48]++; executed_branches++;
                    result_code = 222;  // 温度不在范围
                    statement_coverage[61]++; executed_statements++;
                }
            }
            break;

        case 4:
            branch_coverage[49]++; executed_branches++;
            result_code = 130;
            statement_coverage[62]++; executed_statements++;
            statement_coverage[63]++; executed_statements++;
            if ((temp >= 23 && temp <= 27) && (humidity >= 45 && humidity <= 55) && (key == 4 || key == 0)) {
                branch_coverage[50]++; executed_branches++;
                result_code = 131;
                statement_coverage[64]++; executed_statements++;
            } else {
                branch_coverage[51]++; executed_branches++;
                statement_coverage[65]++; executed_statements++;
                if (temp >= 23 && temp <= 27) {
                    branch_coverage[52]++; executed_branches++;
                    statement_coverage[66]++; executed_statements++;
                    if (humidity >= 45 && humidity <= 55) {
                        branch_coverage[53]++; executed_branches++;
                        result_code = 231;  // 温度湿度OK但按键不对
                        statement_coverage[67]++; executed_statements++;
                    } else {
                        branch_coverage[54]++; executed_branches++;
                        result_code = 232;  // 温度OK但湿度不在范围
                        statement_coverage[68]++; executed_statements++;
                    }
                } else {
                    branch_coverage[55]++; executed_branches++;
                    result_code = 232;  // 温度不在范围
                    statement_coverage[69]++; executed_statements++;
                }
            }
            break;

        default:
            branch_coverage[56]++; executed_branches++;
            statement_coverage[70]++; executed_statements++;
            return 304;
    }
    statement_coverage[71]++; executed_statements++;
    if ((is_daytime == 0 && temp_level == 3) || (is_daytime == 1 && temp_level == 0 && humidity_level == 2)) {
        branch_coverage[57]++; executed_branches++;
        statement_coverage[72]++; executed_statements++;
        if (is_daytime == 0 && temp_level == 3) {
            branch_coverage[58]++; executed_branches++;
            result_code = 403;
            statement_coverage[73]++; executed_statements++;
        } else {
            branch_coverage[59]++; executed_branches++;
            result_code = 404;
            statement_coverage[74]++; executed_statements++;
        }
    } else {
        branch_coverage[60]++; executed_branches++;
        statement_coverage[75]++; executed_statements++;
    }
    statement_coverage[76]++; executed_statements++;
    if ((light_level == 0 && is_daytime == 1) || (light_level == 3 && is_daytime == 0 && temp > 30)) {
        branch_coverage[61]++; executed_branches++;
        statement_coverage[77]++; executed_statements++;
        if (light_level == 0 && is_daytime == 1) {
            branch_coverage[62]++; executed_branches++;
            if (result_code < 200) {
                branch_coverage[63]++; executed_branches++;
                result_code = 205;  // 光强异常警告
                statement_coverage[78]++; executed_statements++;
            } else {
                branch_coverage[64]++; executed_branches++;
                statement_coverage[79]++; executed_statements++;
            }
        } else {
            branch_coverage[65]++; executed_branches++;
            if (result_code < 200) {
                branch_coverage[66]++; executed_branches++;
                result_code = 206;  // 夜间强光异常
                statement_coverage[80]++; executed_statements++;
            } else {
                branch_coverage[67]++; executed_branches++;
                statement_coverage[81]++; executed_statements++;
            }
        }
    } else {
        branch_coverage[68]++; executed_branches++;
        statement_coverage[82]++; executed_statements++;
    }

    statement_coverage[83]++; executed_statements++;
    return result_code;
}

