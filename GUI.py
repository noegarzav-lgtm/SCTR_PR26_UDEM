import sys
import serial 
from collections import deque

from PySide6.QtWidgets import QMainWindow, QApplication, QPushButton, QLineEdit, QWidget, QVBoxLayout, QLabel, QHBoxLayout
from PySide6.QtCore import QTimer
import pyqtgraph as pg
ser = serial.Serial("COM3",115200, timeout = 0.5)

#Declaramos buffers
N = 1000
PV_buffer = deque(maxlen = N)
error_buffer = deque(maxlen = N)
OP_buffer = deque(maxlen = N)
SP_buffer = deque(maxlen = N)

class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Actividad 3: Control de Velocidad con encoder con FreeRTOS")
        #Declaracion del layout madre y container
        container = QWidget()
        layout = QVBoxLayout(container)

        #Plot de velocidad y referencia 
        self.plot1 = pg.PlotWidget(title = "Velocidad y Referencia")
        self.plot1.addLegend()
        self.plot1.showGrid(x = True, y = True)
        self.plot1.setYRange(0, 600)
        self.curve_PV = self.plot1.plot(pen='y', name="PV")
        self.curve_SP = self.plot1.plot(pen='g', name="SP")
        layout.addWidget(self.plot1)

        #Plot de Error y manipulacion
        self.plot2 =pg.PlotWidget(title = "Error y Manipulacion")
        self.plot2.addLegend()
        self.plot2.showGrid(x = True, y = True)
        self.curve_Error = self.plot2.plot(pen='r', name="Error")
        self.curve_OP = self.plot2.plot(pen='w', name="OP")
        layout.addWidget(self.plot2)

        #Declaramos un sub layout para las cajas de despliegue de valores
        values_layout = QHBoxLayout()
        #Cajas de valores de control (PV, OP, Error)
        self.label_PV = QLabel("PV")
        self.value_PV = QLineEdit("0")
        self.value_PV.setReadOnly(True)

        self.label_OP = QLabel("OP")
        self.value_OP = QLineEdit("0")
        self.value_OP.setReadOnly(True)

        self.label_Error = QLabel("Error")
        self.value_Error = QLineEdit("0")
        self.value_Error.setReadOnly(True)

        #Agregamos las cajas a nuestro sub layout
        values_layout.addWidget(self.label_PV)
        values_layout.addWidget(self.value_PV)
        values_layout.addWidget(self.label_Error)
        values_layout.addWidget(self.value_Error)
        values_layout.addWidget(self.label_OP)
        values_layout.addWidget(self.value_OP)

        layout.addLayout(values_layout)

        #layout para valores de PID
        PID_layout = QHBoxLayout()

        self.Kc_value = QLineEdit()
        self.Kc_value.setPlaceholderText("Ingrese Kc")

        self.tao_i_value = QLineEdit()
        self.tao_i_value.setPlaceholderText("Ingrese tao_i")

        self.tao_d_value = QLineEdit("Ingrese tao_d")
        self.tao_d_value.setPlaceholderText("Ingrese tao_d")

        PID_layout.addWidget(self.Kc_value)
        PID_layout.addWidget(self.tao_i_value)
        PID_layout.addWidget(self.tao_d_value)

        layout.addLayout(PID_layout)

        #Boton para mandar Parametros del pid
        boton_PID = QPushButton("Presione aqui para mandar sintonizacion de PID")
        boton_PID.clicked.connect(self.send_PID)
        layout.addWidget(boton_PID)

        #Caja para ingresar set point
        self.setpoint = QLineEdit()
        self.setpoint.setPlaceholderText("Ingrese SP deseado: ")
        layout.addWidget(self.setpoint)

        #Boton para mandar set point
        boton = QPushButton("Da click para enviar SP ingresado")
        boton.clicked.connect(self.send_setpoint)
        boton.clicked.connect(self.enviar_setpoint)
        layout.addWidget(boton)

        self.setCentralWidget(container)

        #Timer para revisar el envio de datos
        self.timer = QTimer()
        self.timer.timeout.connect(self.update_data)
        self.timer.start(100)

    def send_setpoint(self):
        sp = self.setpoint.text()
        ser.write(f"S,{sp},\n".encode())

    def update_data(self):
        try:
            last_line = None

            while ser.in_waiting:
                last_line = ser.readline().decode().strip()

            if last_line:
                PV, OP, Error, SP = map(float, last_line.split(","))

                #Agregamos los valores nuevos al buffer 
                PV_buffer.append(PV)
                OP_buffer.append(OP)
                error_buffer.append(Error)
                SP_buffer.append(SP)

                #Graficar los valores nuevos en el plot
                self.curve_PV.setData(PV_buffer)
                self.curve_OP.setData(OP_buffer)
                self.curve_Error.setData(error_buffer)
                self.curve_SP.setData(SP_buffer)

                #Actualizar valores en lo mostrado del GUI
                self.value_PV.setText(f"{PV:.2f}")
                self.value_Error.setText(f"{Error:.2f}")
                self.value_OP.setText(f"{OP:.2f}")


        except:
            pass

    def enviar_setpoint(self):
        sp = float(self.setpoint.text())
        print("Setpoint a enviar:", sp)   # 👈 Esto lo imprime en la consola de Python
        ser.write(f"SP,{sp}\n".encode())

    def send_PID(self):
        kc = self.Kc_value.text()
        tao_i = self.tao_i_value.text()
        tao_d = self.tao_d_value.text()
        ser.write(f"PID,{kc},{tao_i},{tao_d}\n".encode())


app = QApplication(sys.argv)
window = MainWindow()
window.show()
sys.exit(app.exec())
