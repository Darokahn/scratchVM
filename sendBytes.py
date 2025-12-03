import serial
import time
import argparse
from typing import Optional, List

def interactive_serial_console(port: str, baud_rate: int, timeout: float = 0.1, read_delay: float = 0.05, filename: str = None):
    """
    Connects to a serial port and provides an interactive console 
    to send space-separated decimal byte values and view the response.

    Args:
        port (str): The serial port path (e.g., '/dev/ttyUSB0').
        baud_rate (int): The communication speed (e.g., 115200).
        timeout (float): Serial port read/write timeout in seconds.
        read_delay (float): Short delay after writing before checking for a response.
    """
    
    print(f"--- Interactive Serial Console Initializing ---")
    print(f"Port: {port}")
    print(f"Baud Rate: {baud_rate}")
    
    ser: Optional[serial.Serial] = None
    try:
        # 1. Open and configure the serial port
        # The constructor opens the port and we set a short timeout for reads
        ser = serial.Serial(
            port=port,
            baudrate=baud_rate,
            bytesize=serial.EIGHTBITS,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
            timeout=timeout
        )
        time.sleep(1) # Wait briefly for the port to stabilize
        
        print(f"\nSuccessfully connected to {port}. (Type 'quit' or 'exit' to stop, or Ctrl+C)")
        if (filename):
            with open(filename, "rb") as file:
                ser.write(file.read())
                
        print("-" * 50)
        print("Input Format: Space-separated decimal numbers (0 to 255). Submit empty input to flush received bytes.")
        print("Example: 255 10 0 100")
        print("-" * 50)

        # 2. Main interactive loop
        while True:
            try:
                user_input = input("SEND > ").strip()
                
                if user_input.lower() in ('quit', 'exit'):
                    break

                if user_input:
                    # 3. Parse input into a list of integers (bytes)
                    decimal_strings = user_input.split()
                    byte_list: List[int] = []
                    
                    for s in decimal_strings:
                        number = int(s)
                        if 0 <= number <= 255:
                            byte_list.append(number)
                        else:
                            print(f"Error: Number {number} is out of byte range (0-255). Skipped.")

                    if not byte_list:
                        print("No valid bytes to send.")
                        continue

                    data_to_send = bytes(byte_list)

                    # 4. Send the data
                    bytes_sent = ser.write(data_to_send)
                    print(f"Sent {bytes_sent} bytes: {data_to_send.hex(' ').upper()}")
                    
                    # 5. Wait briefly and read response
                    time.sleep(read_delay)
                    
                if ser.in_waiting > 0:
                    received_data = ser.read(ser.in_waiting)
                    # Print received data as a hex string for clarity
                    print(f"RECV < text: {received_data.decode('utf-8')}")
                    # print(f"RECV < raw text: {received_data}")
                    # print(f"RECV (HEX) < {len(received_data)} bytes: {received_data.hex(' ').upper()}")
                
            except ValueError:
                print("Error: Invalid input. Please ensure all inputs are space-separated decimal numbers (0-255).")
            except serial.SerialTimeoutException:
                print("Warning: Serial write timed out.")
            except KeyboardInterrupt:
                break # Exit gracefully on Ctrl+C

    except serial.SerialException as e:
        print(f"\n[CRITICAL ERROR] Serial Port Error: Could not open or communicate with port {port}.")
        print(f"Details: {e}")
        print("Please check the port path, connection, and permissions.")
    except Exception as e:
        print(f"\n[CRITICAL ERROR] An unexpected error occurred: {e}")
    finally:
        if ser and ser.isOpen():
            ser.close()
            print(f"\nSerial connection to {port} closed. Goodbye!")


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description="Interactive console to send raw bytes over a serial port and receive responses."
    )
    parser.add_argument(
        '--port',
        type=str,
        default='/dev/ttyUSB0',
        help='Serial port name (e.g., /dev/ttyUSB0 or COM3). Defaults to /dev/ttyUSB0.'
    )
    parser.add_argument(
        '--baud',
        type=int,
        default=115200,
        help='Baud rate (e.g., 9600, 115200). Defaults to 115200.'
    )

    parser.add_argument(
        '-r',
        type=str,
        default=None,
        help='filename to use'
    )

    args = parser.parse_args()

    interactive_serial_console(
        port=args.port,
        baud_rate=args.baud,
        filename=args.r
    )
