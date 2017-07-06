""" HardwareInterfaceTest.py

Usage:
    HardwareInterfaceTest.py <port>

"""

import docopt
import logging
import time
import random
import sys
import datetime

import HardwareInterface

def timedelta_to_seconds(td):
    return td.seconds + (td.microseconds/1000000.0)    

class ProgressConsoleHandler(logging.StreamHandler):
    """
    A handler class which allows the cursor to stay on
    one line for selected messages
    https://stackoverflow.com/questions/3118059/how-to-write-custom-python-logging-handler
    """
    on_same_line = False
    def emit(self, record):
        try:
            msg = self.format(record)
            stream = self.stream
            same_line = hasattr(record, 'same_line')
            if self.on_same_line and not same_line:
                stream.write(self.terminator)
            stream.write(msg)
            if same_line:
                self.on_same_line = True
            else:
                stream.write(self.terminator)
                self.on_same_line = False
            self.flush()
        except (KeyboardInterrupt, SystemExit):
            raise
        except:
            self.handleError(record)

def get_logger():
    return logging.getLogger(__name__)

def run_set_timestamp_test(interface, logger):
    logger.info("Setting timestamp...")

    response, expected = interface.set_timestamp()

    if response.strip() != "OK":
        logger.info("Timestamp set failed.")
        sys.exit(1)

    logger.info("Getting timestamp...")

    actual = interface.get_timestamp()

    diff = abs(actual - expected)

    logger.info("Expected {} got {} (delta = {}ms)".format(expected, actual, diff.microseconds/1000))

    if diff.seconds > 1:
        logger.error("Got delta > 1s ({}d, {}.{}s)".format(diff.days, diff.seconds, diff.microseconds))
        sys.exit(1)

def run_pulse_test(interface, logger):

    logger.info("Sending pulses...")
    expected_timestamps = []

    for _ in range(10):
        response = interface.fake_pulse()
        expected_timestamps.append(datetime.datetime.now())

        if response.strip() != "OK":
            logger.info("Send pulse failed.")
            sys.exit(1)

        get_logger().info('.', extra={'same_line':True})
        time.sleep(random.randint(500, 2500)/1000.0)

    logger.info("Getting count...")

    count = int(interface.get_pulse_count())

    if count != 10:
        logger.error("Expected 10 pulses, got %d", count)
        sys.exit()

    logger.info("Getting timestamps...")

    actual_timestamps = [interface.get_pulse_time() for _ in range(10)]
    
    for expected, actual in zip(expected_timestamps, actual_timestamps):
        diff = abs(actual - expected)
        logger.info("Expected {} got {} (delta = {}ms)".format(expected, actual, (diff.seconds*1000)+(diff.microseconds/1000)))

    actual = interface.get_pulse_time()
    if actual is not None:
        logger.info("Expected no timestamp, got {}".format(expected, actual, diff.microseconds/1000))

    logger.info("Calculating intervals...")
    actual_intervals = [abs(t1-t2) for t1,  t2 in zip(actual_timestamps[0:-1], actual_timestamps[1:])]
    expected_intervals = [abs(t1-t2) for t1,  t2 in zip(expected_timestamps[0:-1], expected_timestamps[1:])]

    for expected, actual in zip(expected_intervals, actual_intervals):
        actual_seconds = timedelta_to_seconds(actual)
        logger.info("Expected {}s got {}s".format(expected, actual_seconds))

def set_initial_timestamp(interface):
    interface.set_timestamp()

def run_timestamp_tests(interface, test_logger):
    get_logger().info("Running set/get timestamp tests...")

    for _ in range(0, 100):
        run_set_timestamp_test(interface, test_logger)
        get_logger().info('.', extra={'same_line':True})
        time.sleep(random.randint(1,50)/1000.0)

    get_logger().info("Done")

def run_pulse_tests(interface, test_logger):

    get_logger().info("Running pulse tests...".format(port))

    run_pulse_test(interface, test_logger)

    get_logger().info("Done")    

def run_clock_divergence_test(interface, test_logger):

    get_logger().info("Running timestamp divergence test...".format(port))

    _, actual_start = interface.set_timestamp()
    start = datetime.datetime.now()

    time.sleep(10)

    end = datetime.datetime.now()
    actual_end = interface.get_timestamp()

    diff = end-start
    actual_diff = actual_end - actual_start

    get_logger().info(
        "Expected diff {}s, actual diff {}s".format(
            timedelta_to_seconds(diff), timedelta_to_seconds(actual_diff)
        )
    )
    get_logger().info("Done")  

if __name__ == "__main__":

    random.seed(0)

    progress = ProgressConsoleHandler()

    get_logger().setLevel(logging.INFO)
    get_logger().addHandler(progress)

    test_logger = logging.getLogger("{}.{}".format(__name__, "test_logger"))
    test_logger.setLevel(logging.INFO)

    args = docopt.docopt(__doc__)
    port = args["<port>"]
    
    get_logger().info("Running hardware tests on {}".format(port))

    interface = HardwareInterface.HardwareInterface(port)

    set_initial_timestamp(interface)

    run_timestamp_tests(interface, test_logger)

    run_clock_divergence_test(interface, test_logger)

    run_pulse_tests(interface, test_logger)
