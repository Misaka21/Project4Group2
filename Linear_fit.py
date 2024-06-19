import yaml
import numpy as np
import matplotlib.pyplot as plt
from sklearn.linear_model import LinearRegression

def load_calibration_data(file_path):
    with open(file_path, 'r') as file:
        calibration_data = yaml.safe_load(file)
    return calibration_data

# 示例使用
data = load_calibration_data('calibration.yaml')

camera_0 = []
arm_0 = []
camera_1 = []
arm_1 = []

for item in data['Calibration Data']:
    camera_0.append(item['Camera'][0])
    arm_0.append(item['Arm'][0])
    camera_1.append(item['Camera'][1])
    arm_1.append(item['Arm'][1])

# 将数据转换为NumPy数组
camera_0 = np.array(camera_0).reshape(-1, 1)
arm_0 = np.array(arm_0).reshape(-1, 1)
camera_1 = np.array(camera_1).reshape(-1, 1)
arm_1 = np.array(arm_1).reshape(-1, 1)

# 拟合线性回归模型
model_0 = LinearRegression().fit(camera_0, arm_0)
model_1 = LinearRegression().fit(camera_1, arm_1)

# 生成拟合线
x_0 = np.linspace(min(camera_0), max(camera_0), 100).reshape(-1, 1)
y_0 = model_0.predict(x_0)

x_1 = np.linspace(min(camera_1), max(camera_1), 100).reshape(-1, 1)
y_1 = model_1.predict(x_1)

# 绘制散点图和拟合线
plt.figure(figsize=(10, 6))

# Camera 0 和 Arm 0
plt.scatter(camera_0, arm_0, color='blue', label='Camera 0 vs Arm 0')
plt.plot(x_0, y_0, color='blue', linestyle='--', label=f'Fit: y = {model_0.coef_[0][0]:.2f}x + {model_0.intercept_[0]:.2f}')

# Camera 1 和 Arm 1
plt.scatter(camera_1, arm_1, color='red', label='Camera 1 vs Arm 1')
plt.plot(x_1, y_1, color='red', linestyle='--', label=f'Fit: y = {model_1.coef_[0][0]:.2f}x + {model_1.intercept_[0]:.2f}')

# 添加标题和标签
plt.title('Camera and Arm Calibration Data')
plt.xlabel('Camera')
plt.ylabel('Arm')
plt.legend()
plt.grid(True)
plt.show()
