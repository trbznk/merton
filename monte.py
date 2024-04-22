import ctypes

from PySide6.QtCore import Qt
from PySide6.QtWidgets import QApplication, QWidget, QPushButton, QVBoxLayout, QLineEdit, QLabel, QGridLayout, QGroupBox
from PySide6.QtCharts import QChartView, QChart

class Config(ctypes.Structure):
    _fields_ = [
        ("asset_corr", ctypes.c_float),
        ("cut_off", ctypes.c_float),
        ("EAD", ctypes.c_float),
        ("LGD", ctypes.c_float),
        ("pf_size", ctypes.c_int),
        ("n_sims", ctypes.c_int),
    ]

class Result(ctypes.Structure):
    _fields_ = [
        ("L_PF", ctypes.POINTER(ctypes.c_float))
    ]

class Window(QWidget):
    def __init__(self):
        super().__init__()

        main_layout = QGridLayout()
        main_layout.setColumnStretch(1, 10)

        input_groupbox = QGroupBox("Configuration")
        self.asset_corr_input = QLineEdit()
        self.cut_off_input = QLineEdit()
        self.ead_input = QLineEdit()
        self.lgd_input = QLineEdit()
        self.pf_size_input = QLineEdit()
        self.n_sims_input = QLineEdit()
        self.simulate_button = QPushButton("Simulate")

        input_layout = QGridLayout()
        input_groupbox.setLayout(input_layout)
        input_layout.addWidget(QLabel("Asset correlation:"), 0, 0, Qt.AlignRight)
        input_layout.addWidget(self.asset_corr_input, 0, 1)
        input_layout.addWidget(QLabel("Cut-Off-Value:"), 1, 0, Qt.AlignRight)
        input_layout.addWidget(self.cut_off_input, 1, 1)
        input_layout.addWidget(QLabel("EAD:"), 2, 0, Qt.AlignRight)
        input_layout.addWidget(self.ead_input, 2, 1)
        input_layout.addWidget(QLabel("LGD:"), 3, 0, Qt.AlignRight)
        input_layout.addWidget(self.lgd_input, 3, 1)
        input_layout.addWidget(QLabel("Portfolio size:"), 4, 0, Qt.AlignRight)
        input_layout.addWidget(self.pf_size_input, 4, 1)
        input_layout.addWidget(QLabel("Simulations:"), 5, 0, Qt.AlignRight)
        input_layout.addWidget(self.n_sims_input, 5, 1)
        input_layout.addWidget(self.simulate_button, 6, 1)

        result_groupbox = QGroupBox("Result")
        result_layout = QGridLayout()
        result_groupbox.setLayout(result_layout)
        result_layout.addWidget(QLabel("L_PF:"), 0, 0)

        plotting_groupbox = QGroupBox("Plotting")
        plotting_layout = QVBoxLayout()
        plotting_groupbox.setLayout(plotting_layout)
        chart_view = QChartView()
        plotting_layout.addWidget(chart_view)

        main_layout.addWidget(input_groupbox, 0, 0)
        main_layout.addWidget(result_groupbox, 1, 0)
        main_layout.addWidget(plotting_groupbox, 0, 1, 2, 1)

        self.setWindowTitle("merton")
        self.setLayout(main_layout)

libmonte = ctypes.CDLL("./monte.so")
libmonte.simulate.argtypes = [Config, Result]
libmonte.simulate.restype = Result

result = Result((ctypes.c_float*1000)())
config = Config(0.05, -2.0, 1.0, 0.6, 100, 1000)
result = libmonte.simulate(config, result)

# print(result)
# for i in range(10):
#     print(result.L_PF[i])

app = QApplication()
window = Window()
window.show()
app.exec()