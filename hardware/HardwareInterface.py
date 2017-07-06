import serial
import datetime
import time
import sys

class HardwareInterface:

    def __init__(self, port, baudrate=115200):
        self.port = serial.Serial(port, baudrate, timeout=2)
        self.port.readline()

    @staticmethod
    def get_set_timestamp_command_bytes(timestamp):
        return "!T{}\n".format(timestamp).encode("ascii")

    def set_timestamp(self, timestamp_dt=None, debug=False):

        timestamp_dt = timestamp_dt or datetime.datetime.now()

        timestamp = "{:%H:%M:%S.%f}".format(timestamp_dt)
        timestamp = timestamp[:-3]

        command = self.get_set_timestamp_command_bytes(timestamp)

        if debug:
            print("Sending {}".format(command.decode("ascii")))

        self.port.write(command)

        resp = self.port.readline()

        if debug:
            print("Got {}".format(resp))

        return resp.decode("ascii"), timestamp_dt

    def get_timestamp(self, debug=False):
        command = "?T\n".encode("ascii")

        if debug:
            print("Sending {}".format(command.decode("ascii")))

        self.port.write(command)

        resp = self.port.readline().decode("ascii").strip()

        if debug:
            print("Got {}".format(resp))

        return datetime.datetime.combine(
            datetime.datetime.today(),
            datetime.datetime.strptime(resp, "%H:%M:%S.%f").time()
        )

    def get_pulse_time(self, debug=False):

        command = "?I\n".encode("ascii")

        if debug:
            print("Sending {}".format(command.decode("ascii")))

        self.port.write(command)
        resp = self.port.readline().decode("ascii").strip()

        if debug:
            print("Got {}".format(resp))

        if resp != "--":
            return datetime.datetime.combine(
                datetime.datetime.today(),
                datetime.datetime.strptime(resp, "%H:%M:%S.%f").time()
            )
        else:
            return None

    def fake_pulse(self, debug=False):
        command = "!F\n".encode("ascii")

        if debug:
            print("Sending {}".format(command.decode("ascii")))

        self.port.write(command)
 
        resp = self.port.readline().decode("ascii").strip()

        if debug:
            print("Got {}".format(resp))

        return resp

    def get_pulse_count(self, debug=False):
        command = "?C\n".encode("ascii")

        if debug:
            print("Sending {}".format(command.decode("ascii")))

        self.port.write(command)

        resp = self.port.readline().decode("ascii").strip()

        if debug:
            print("Got {}".format(resp))

        return resp
