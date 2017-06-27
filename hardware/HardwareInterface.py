import serial
import datetime
import time
import sys

class HardwareInterface:

    def __init__(self, port, baudrate=115200):
        self.port = serial.Serial(port, baudrate)
        self.port.readline()

    @staticmethod
    def get_set_timestamp_command_bytes(timestamp):
        return "!T{}\n".format(timestamp).encode("ascii")

    def set_timestamp(self, timestamp_dt=None):

        timestamp_dt = timestamp_dt or datetime.datetime.now()

        timestamp = "{:%H:%M:%S.%f}".format(timestamp_dt)
        timestamp = timestamp[:-3]

        command = self.get_set_timestamp_command_bytes(timestamp)

        self.port.write(command)

        resp = self.port.readline()

        return resp.decode("ascii"), timestamp_dt

    def get_timestamp(self):
        command = "?T\n".encode("ascii")
        self.port.write(command)
        response = self.port.readline().decode("ascii").strip()

        return datetime.datetime.combine(
            datetime.datetime.today(),
            datetime.datetime.strptime(response, "%H:%M:%S.%f").time()
        )

    def get_pulse_time(self):

        command = "?I\n".encode("ascii")
        self.port.write(command)
        response = self.port.readline().decode("ascii").strip()

        if response != "--":
            return datetime.datetime.combine(
                datetime.datetime.today(),
                datetime.datetime.strptime(response, "%H:%M:%S.%f").time()
            )
        else:
            return None

    def fake_pulse(self):
        command = "!F\n".encode("ascii")
        self.port.write(command)
        return self.port.readline().decode("ascii").strip()

    def get_pulse_count(self):
        command = "?C\n".encode("ascii")
        self.port.write(command)
        return self.port.readline().decode("ascii").strip()

