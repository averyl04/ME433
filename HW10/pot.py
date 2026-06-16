import serial
import pgzrun

WIDTH = 800
HEIGHT = 400

ser = serial.Serial("COM5", 115200, timeout=0.01)

pot_value = 0


def update():

    global pot_value

    if ser.in_waiting:

        try:
            line = ser.readline().decode().strip()

            if line != "":
                pot_value = int(line)

        except:
            pass


def draw():

    screen.clear()

    screen.draw.text(
        f"Potentiometer: {pot_value}",
        (20, 20),
        fontsize=40
    )

    x = int((pot_value / 65535) * WIDTH)

    screen.draw.filled_circle(
        (x, HEIGHT // 2),
        40,
        "red"
    )


pgzrun.go()