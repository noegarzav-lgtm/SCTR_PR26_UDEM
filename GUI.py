import sys
import serial 
from collections import deque

#Librerias de Qt desde PySide6
from PySide6.QtWidgets import QMainWindow, QApplication, QPushButton, QLineEdit, QWidget, QVBoxLayout, QLabel, QHBoxLayout
from PySide6.QtCore import QTimer
import pyqtgraph as pg

#Declaramos puerto serial a utilizar
ser = serial.Serial("COM3",115200, timeout = 0.5)

#Declaramos buffers y su tamaño
N = 1000
PV_buffer = deque(maxlen = N)
error_buffer = deque(maxlen = N)
OP_buffer = deque(maxlen = N)
SP_buffer = deque(maxlen = N)

class MainWindow(QMainWindow):
    #Declaramos funcion de inicializacion de ventana y Widgets
    def __init__(self):
        super().__init__()
        #Declaracion de nuestra ventana y su titulo
        self.setWindowTitle("Actividad 3: Control de Velocidad con encoder con FreeRTOS")

        #Declaracion del layout madre y container
        container = QWidget()
        main_layout = QHBoxLayout(container)
        #Declaramos dos sublayouts uno de lado izquierdo y otro de derecho
        left_layout = QVBoxLayout()
        right_layout = QHBoxLayout()

        #Plot de velocidad y referencia 
        self.plot1 = pg.PlotWidget(title = "Velocidad y Referencia")
        self.plot1.addLegend()
        self.plot1.showGrid(x = True, y = True)
        self.plot1.setYRange(0, 600)
        #Creacion de las curvas para el plot
        self.curve_PV = self.plot1.plot(pen='y', name="PV")
        self.curve_SP = self.plot1.plot(pen='g', name="SP")
        #Agregamos nuestro plot al layout izquierdo
        left_layout.addWidget(self.plot1)

        #Plot de Error y manipulacion
        self.plot2 =pg.PlotWidget(title = "Error y Manipulacion")
        self.plot2.addLegend()
        self.plot2.showGrid(x = True, y = True)
        #Creacion de las curvas para el plot
        self.curve_Error = self.plot2.plot(pen='r', name="Error")
        self.curve_OP = self.plot2.plot(pen='w', name="OP")
        #Agregamos nuestro plot al layout izquierdo
        left_layout.addWidget(self.plot2)

        #Declaramos un subsub layout para las cajas de despliegue de valores
        values_layout = QHBoxLayout()
        #Cajas de valores de control (PV, OP, Error)
        self.label_PV = QLabel("PV")
        self.value_PV = QLineEdit("0")
        self.value_PV.setReadOnly(True) #Este comando asegura que nuestro LineEdit no sea modificable, siendo asi que solo muestra valores

        self.label_OP = QLabel("OP")
        self.value_OP = QLineEdit("0")
        self.value_OP.setReadOnly(True)

        self.label_Error = QLabel("Error")
        self.value_Error = QLineEdit("0")
        self.value_Error.setReadOnly(True)

        #Agregamos las cajas a nuestro subsub layout
        values_layout.addWidget(self.label_PV)
        values_layout.addWidget(self.value_PV)
        values_layout.addWidget(self.label_Error)
        values_layout.addWidget(self.value_Error)
        values_layout.addWidget(self.label_OP)
        values_layout.addWidget(self.value_OP)

        #Agregamos el subsub layout de valores al sub layout izquierdo
        left_layout.addLayout(values_layout)

        #layout para valores de PID
        PID_layout = QHBoxLayout()
        #Declaramos cajas para ingresar valores de los parametros PID
        self.Kc_value = QLineEdit()
        self.Kc_value.setPlaceholderText("Ingrese Kc")

        self.tao_i_value = QLineEdit()
        self.tao_i_value.setPlaceholderText("Ingrese tao_i")

        self.tao_d_value = QLineEdit()
        self.tao_d_value.setPlaceholderText("Ingrese tao_d")

        #Agregamos las cajas a nuestro layout de PID
        PID_layout.addWidget(self.Kc_value)
        PID_layout.addWidget(self.tao_i_value)
        PID_layout.addWidget(self.tao_d_value)

        #Agregamos el layout del PID al layout izquierdo
        left_layout.addLayout(PID_layout)

        #Boton para mandar Parametros del pid
        boton_PID = QPushButton("Presione aqui para mandar sintonizacion de PID")
        boton_PID.clicked.connect(self.send_PID)
        left_layout.addWidget(boton_PID)

        #Caja para ingresar set point
        self.setpoint = QLineEdit()
        self.setpoint.setPlaceholderText("Ingrese SP deseado: ")
        left_layout.addWidget(self.setpoint)

        #Boton para mandar set point
        boton = QPushButton("Da click para enviar SP ingresado")
        boton.clicked.connect(self.send_setpoint)
        left_layout.addWidget(boton)


        #Layout de latencia
        lat_layout = QVBoxLayout()

        #Cajas de valores de latencia
        self.label_pidL = QLabel("Latenica PID")
        self.value_pidL = QLineEdit("0")
        self.value_pidL.setReadOnly(True)

        self.label_pwmL = QLabel("Latenica PWM")
        self.value_pwmL = QLineEdit("0")
        self.value_pwmL.setReadOnly(True)

        self.label_speedL = QLabel("Latenica Velocidad")
        self.value_speedL = QLineEdit("0")
        self.value_speedL.setReadOnly(True)

        self.label_guiL = QLabel("Latenica GUI")
        self.value_guiL = QLineEdit("0")
        self.value_guiL.setReadOnly(True)

        #Argegamos Widgets al layout de latencia
        lat_layout.addWidget(self.label_pidL)
        lat_layout.addWidget(self.value_pidL)

        lat_layout.addWidget(self.label_pwmL)
        lat_layout.addWidget(self.value_pwmL)

        lat_layout.addWidget(self.label_speedL)
        lat_layout.addWidget(self.value_speedL)

        lat_layout.addWidget(self.label_guiL)
        lat_layout.addWidget(self.value_guiL)

        #Agregamos el layout de latencia al lado derecho
        right_layout.addLayout(lat_layout)

        #Layout de jitter
        jit_layout = QVBoxLayout()
        #Cajas de valores de jitter
        self.label_pidJ = QLabel("Jitter PID")
        self.value_pidJ = QLineEdit("0")
        self.value_pidJ.setReadOnly(True)

        self.label_pwmJ = QLabel("Jitter PWM")
        self.value_pwmJ = QLineEdit("0")
        self.value_pwmJ.setReadOnly(True)

        self.label_speedJ = QLabel("Jitter Velocidad")
        self.value_speedJ = QLineEdit("0")
        self.value_speedJ.setReadOnly(True)

        self.label_guiJ = QLabel("Jitter GUI")
        self.value_guiJ = QLineEdit("0")
        self.value_guiJ.setReadOnly(True)

        #Agregamos cajas al layout del jitter
        jit_layout.addWidget(self.label_pidJ)
        jit_layout.addWidget(self.value_pidJ)

        jit_layout.addWidget(self.label_pwmJ)
        jit_layout.addWidget(self.value_pwmJ)

        jit_layout.addWidget(self.label_speedJ)
        jit_layout.addWidget(self.value_speedJ)

        jit_layout.addWidget(self.label_guiJ)
        jit_layout.addWidget(self.value_guiJ)

        #Agregamos layout del jitter al lado derecho
        right_layout.addLayout(jit_layout)

        #Layout de periodo real
        periodo_layout = QVBoxLayout()
        #Declaramos Widgets de los periodos reales
        self.label_pidP = QLabel("Periodo Real PID")
        self.value_pidP = QLineEdit("0")
        self.value_pidP.setReadOnly(True)

        self.label_pwmP = QLabel("Periodo Real PWM")
        self.value_pwmP = QLineEdit("0")
        self.value_pwmP.setReadOnly(True)

        self.label_speedP = QLabel("Periodo Real Velocidad")
        self.value_speedP = QLineEdit("0")
        self.value_speedP.setReadOnly(True)

        self.label_guiP = QLabel("Periodo Real GUI")
        self.value_guiP = QLineEdit("0")
        self.value_guiP.setReadOnly(True)

        #Agregamos Widgets de periodo real al layout de periodo real
        periodo_layout.addWidget(self.label_pidP)
        periodo_layout.addWidget(self.value_pidP)

        periodo_layout.addWidget(self.label_pwmP)
        periodo_layout.addWidget(self.value_pwmP)

        periodo_layout.addWidget(self.label_speedP)
        periodo_layout.addWidget(self.value_speedP)

        periodo_layout.addWidget(self.label_guiP)
        periodo_layout.addWidget(self.value_guiP)

        #Agregamos el layout de periodos reales a la derecha
        right_layout.addLayout(periodo_layout)

        #Agregamos layouts izquierdo y derecho a nuestro layout principal
        main_layout.addLayout(left_layout,3)
        main_layout.addLayout(right_layout,1)
        #Contender central
        self.setCentralWidget(container)

        #Timer para revisar el envio de datos
        self.timer = QTimer()
        self.timer.timeout.connect(self.update_data)
        self.timer.start(100)

    def send_setpoint(self):
        #Mandamos nuestro setpoint por puerto serial
        sp = self.setpoint.text()
        ser.write(f"S,{sp},\n".encode())

    def update_data(self):
        try:
            #Inicializamos nuestra variable para guardar la infomracion
            last_line = None

            while ser.in_waiting:
                #Decodificamos los Serial.print hechos en GUI.cpp
                last_line = ser.readline().decode().strip()

            if last_line:
                #Asignamos cada valor correspondiente a una variable local 
                PV, OP, Error, SP, pid_periodo_real ,pid_latencia, pid_jitter, pwm_periodo_real, pwm_latencia, pwm_jitter, speed_periodo_real, speed_latencia, speed_jitter, gui_periodo_real, gui_latencia, gui_jitter = map(float, last_line.split(","))

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
                
                #Actualizar valores de periodo real, jitter y latencia
                self.value_pidP.setText(f"{pid_periodo_real}")
                self.value_pidL.setText(f"{pid_latencia}")
                self.value_pidJ.setText(f"{pid_jitter}")

                self.value_pwmP.setText(f"{pwm_periodo_real}")
                self.value_pwmL.setText(f"{pwm_latencia}")
                self.value_pwmJ.setText(f"{pwm_jitter}")

                self.value_speedP.setText(f"{speed_periodo_real}")
                self.value_speedL.setText(f"{speed_latencia}")
                self.value_speedJ.setText(f"{speed_jitter}")

                self.value_guiP.setText(f"{gui_periodo_real}")
                self.value_guiL.setText(f"{gui_latencia}")
                self.value_guiJ.setText(f"{gui_jitter}")
        except:
            pass
    def send_PID(self):
        #Mandamos los valores de PID al ESP por puerto serial
        kc = self.Kc_value.text()
        tao_i = self.tao_i_value.text()
        tao_d = self.tao_d_value.text()
        ser.write(f"PID,{kc},{tao_i},{tao_d}\n".encode())

#Creamos nuestra ventana y la mostramos
app = QApplication(sys.argv)
window = MainWindow()
window.show()
sys.exit(app.exec())
