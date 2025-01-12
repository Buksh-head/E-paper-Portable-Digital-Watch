import tkinter as tk
from tkinter import messagebox
from tkinter import ttk
from datetime import datetime, timedelta
import requests
import os
import time
import json
import serial
import serial.tools.list_ports


# Global variables
time_interval = 60000
flashing_active = False
scheduled_task_id = None
weather_data = {}
temperature_unit = "C"  # Default to Celsius
accelerated_mode = 0  # 0: normal, 1: mode 1, 2: mode 2
start_time = datetime.now()
flashing = False
data_file = "weather_data.json"
alarms = []


def load_weather_data():
    global weather_data
    if os.path.exists(data_file):
        with open(data_file, 'r') as file:
            weather_data = json.load(file)
            print("Weather data loaded from file:", weather_data)


def save_weather_data():
    """Save weather data to a local JSON file."""
    global weather_data
    with open(data_file, 'w') as file:
        json.dump(weather_data, file)
        print("Weather data saved to file")


def update_weather_data():
    """Fetch the latest weather data from the server and update the display."""
    global weather_data
    try:
        print("Fetching weather data...")
        response = requests.get("https://tp-weather.uqcloud.net/weather.json")
        if response.status_code == 200:
            weather_data = response.json()
            save_weather_data()
            display_weather_data()
            print("Weather data fetched and displayed successfully.")
        else:
            handle_fetch_error("Failed to fetch weather data.")
    except Exception as e:
        handle_fetch_error(f"An error occurred: {e}")


def handle_fetch_error(message):
    """Handle errors during weather data fetch."""
    messagebox.showerror("Error", message)
    print("Error:", message)


def display_weather_data():
    """Display the current and next 7 days' weather data on the GUI."""
    global temperature_unit
    today = start_time.date()
    today_formatted = today.strftime("%Y-%m-%d")
    day_of_week = today.strftime("%A")
    print(f"Displaying weather for today: {today_formatted} - {day_of_week}")

    if 'weather' in weather_data:
        print("Weather data is available.")
        weather_today = weather_data['weather'].get(today_formatted)
        if weather_today:
            temperature = convert_temperature(weather_today["temperature"])
            humidity = weather_today["humidity"]
            forecast = weather_today["forecast"]
            print(f"Today's weather data: {weather_today}")
        else:
            # No weather data for today, use default values
            temperature = 0
            humidity = 0
            forecast = "No"
            print("No weather data available for today. Using default values.")

        current_weather_label.config(
            text=f"{day_of_week}: {forecast.capitalize()}, {temperature:.1f}°{temperature_unit}, {humidity}% humidity"
        )
        populate_forecast_listbox()
    else:
        # No weather data in the loaded data, use default values
        print("No weather data found in the loaded data. Using default values.")
        current_weather_label.config(
            text=f"{day_of_week}: No data, 0.0°{temperature_unit}, 0% humidity"
        )


def convert_temperature(temp_celsius):
    """Convert temperature from Celsius to Fahrenheit if required."""
    if temperature_unit == "F":
        return (temp_celsius * 9/5) + 32
    return temp_celsius


def populate_forecast_listbox():
    """Populate the forecast listbox with weather data for the next 7 days."""
    forecast_listbox.delete(0, tk.END)
    if 'weather' in weather_data:
        for i in range(7):
            date = (start_time + timedelta(days=i)).strftime("%Y-%m-%d")
            date_parts = date.split('-')
            date_formatted = f"{date_parts[0]}-{int(date_parts[1])}-{int(date_parts[2])}"
            daily_weather = weather_data['weather'].get(date_formatted)

            if daily_weather:
                temp = convert_temperature(daily_weather["temperature"])
                hum = daily_weather["humidity"]
                fore = daily_weather["forecast"]
            else:
                # No weather data, use default values
                temp = 0
                hum = 0
                fore = "No Data"
                print(
                    f"No weather data for {date_formatted}. Using default values.")

            forecast_listbox.insert(
                tk.END, f"{date_formatted}: {fore.capitalize()}, {temp:.1f}°{temperature_unit}, {hum}% humidity"
            )
            print(
                f"Forecast for {date_formatted}: {fore.capitalize()}, {temp:.1f}°{temperature_unit}, {hum}% humidity"
            )
    else:
        # No weather data at all, fill with default values
        print("No forecast data available. Filling with default values.")
        for i in range(7):
            date = (start_time + timedelta(days=i)).strftime("%Y-%m-%d")
            forecast_listbox.insert(
                tk.END, f"{date}: No data, 0.0°{temperature_unit}, 0% humidity"
            )


def update_weather_unit():
    """Update the temperature unit and refresh the weather display."""
    global temperature_unit
    temperature_unit = weather_format_var.get()
    display_weather_data()


def display_time():
    """Continuously display the current system time in the selected format, updating every minute."""
    global accelerated_mode, start_time
    current_time = start_time
    # Display time in the selected format, excluding seconds
    if time_format_var.get() == "12-hour":
        time_string = current_time.strftime("%I:%M %p")
    else:
        time_string = current_time.strftime("%H:%M")

    date_string = current_time.strftime("%d/%m/%Y")
    # Update the time display in the GUI
    current_time_display.config(
        text=time_string + " " + date_string, font=("Arial", 16))


def increment_time():
    global start_time, accelerated_mode, time_interval, scheduled_task_id
    # Calculate the current time based on the selected mode
    if accelerated_mode == 0:
        current_time = start_time + timedelta(seconds=60)
    elif accelerated_mode == 1:
        current_time = start_time + timedelta(seconds=60)
    elif accelerated_mode == 2:
        current_time = start_time + timedelta(hours=1)

    if time_format_var.get() == "12-hour":
        time_string = current_time.strftime("%I:%M %p")
    else:
        time_string = current_time.strftime("%H:%M")

    date_string = current_time.strftime("%d/%m/%Y")
    current_time_display.config(
        text=time_string + " " + date_string, font=("Arial", 16))

    start_time = current_time
    scheduled_task_id = root.after(time_interval, increment_time)


def set_time():
    """Set the time manually based on user input."""
    global start_time
    manual_time = manual_time_entry.get()
    try:
        # Pause automatic updates by stopping the after loop temporarily
        root.after_cancel(display_time)

        # Parse the manual time entry
        if time_format_var.get() == "12-hour":
            entered_time = datetime.strptime(manual_time, '%I:%M %p %d/%m/%Y')
        else:
            entered_time = datetime.strptime(manual_time, '%H:%M %d/%m/%Y')

        # Update the current time display with the entered time
        current_time_display.config(text=manual_time, font=("Arial", 16))
        # Set the global start_time to the entered time
        start_time = entered_time
        messagebox.showinfo("Success", "Time set successfully!")
    except ValueError:
        messagebox.showerror(
            "Error", "Invalid time format. Please use the correct format for the selected time mode.")


def switch_time_format():
    if time_format_var.get() == "12-hour":
        time_format_var.set("24-hour")
        Radiobutton_12h.config(text="Switch to 12-hour")
    else:
        time_format_var.set("12-hour")
        Radiobutton_24h.config(text="Switch to 24-hour")


def set_system_time():
    """Set the system time and start continuous display."""
    global start_time
    start_time = datetime.now()
    # Format the time to display in the entry box
    if time_format_var.get() == "12-hour":
        time_string = start_time.strftime('%I:%M %p')
    else:
        time_string = start_time.strftime('%H:%M')

    date_string = start_time.strftime('%d/%m/%Y')
    # Populate the manual time entry box
    manual_time_entry.delete(0, tk.END)
    manual_time_entry.insert(0, f"{time_string} {date_string}")


def set_normal_mode():
    """Set the device to normal time mode."""
    global accelerated_mode, time_interval, scheduled_task_id
    accelerated_mode = 0
    time_interval = 60000
    if scheduled_task_id is not None:
        root.after_cancel(scheduled_task_id)
    scheduled_task_id = root.after(time_interval, increment_time)


def set_accelerated_mode1():
    """Set the device to accelerated mode 1."""
    global accelerated_mode, time_interval, scheduled_task_id
    accelerated_mode = 1
    time_interval = 4000
    if scheduled_task_id is not None:
        root.after_cancel(scheduled_task_id)
    scheduled_task_id = root.after(time_interval, increment_time)


def set_accelerated_mode2():
    """Set the device to accelerated mode 2."""
    global accelerated_mode, time_interval, scheduled_task_id
    accelerated_mode = 2
    time_interval = 1000
    if scheduled_task_id is not None:
        root.after_cancel(scheduled_task_id)
    scheduled_task_id = root.after(time_interval, increment_time)


def trigger_alarm():
    global ser
    # Send a command to the microcontroller to disable sleep mode and wake up the device
    if ser and ser.is_open:
        try:
            # Send alarm signal to wake up the device
            ser.write(b"ALARM_TRIGGER\n")
            print("Alarm triggered, waking up the device.")
            ser.flush()
        except Exception as e:
            print(f"Error sending alarm trigger: {e}")


def check_alarms(current_time):
    """Check if any alarms should trigger and wake the microcontroller."""
    for alarm in alarms:
        if alarm and alarm['time'] <= current_time:
            trigger_alarm()
            break


def add_update_alarm():
    """Add or update an alarm based on user input."""
    if len(alarms) == 2 and alarms[0] is not None and alarms[1] is not None:
        messagebox.showerror("Error", "Only 2 alarms are allowed.")
        return
    alarm_time = time_entry.get()  # Get the time from the entry
    # Get the message from the text widget
    alarm_message = message_entry.get("1.0", tk.END).strip()
    led_blink = led_var.get()  # LED blink status
    buzzer = buzzer_var.get()  # Buzzer status

    if not alarm_time or not alarm_message:
        messagebox.showerror(
            "Error", "Please enter both time and message for the alarm.")
        return

    # Convert the entered alarm time to a datetime object for comparison
    alarm_time_obj = datetime.strptime(alarm_time, '%H:%M %d/%m/%Y')

    # Create a dictionary for the alarm
    alarm = {
        "time": alarm_time,
        "message": alarm_message,
        "led": led_blink,
        "buzzer": buzzer
    }

    # Add or update the alarm in the list
    if len(alarms) == 0:
        alarms.append(alarm)
    elif alarms[0] is None:
        alarms[0] = alarm
    elif len(alarms) == 1:
        alarms.append(alarm)
    else:
        alarms[1] = alarm
    update_alarm_listbox()

    # Clear the entries after adding/updating
    time_entry.delete(0, tk.END)
    message_entry.delete("1.0", tk.END)
    led_var.set(False)
    buzzer_var.set(False)

    messagebox.showinfo("Success", "Alarm added/updated successfully.")


def delete_alarm():
    """Delete the selected alarm from the listbox and alarm list."""
    selected_index = alarm_listbox.curselection()
    if not selected_index:
        messagebox.showerror("Error", "Please select an alarm to delete.")
        return

    # Remove the selected alarm from the list and update the display
    alarms[selected_index[0]] = None
    update_alarm_listbox(deleted=selected_index[0])

    messagebox.showinfo("Success", "Alarm deleted successfully.")


def update_alarm_listbox(deleted=None):
    """Update the alarm listbox to display current alarms."""
    alarm_listbox.delete(0, tk.END)
    if deleted == "both":
        alarm_one.config(text="")
        alarm_two.config(text="")

    elif deleted is not None:
        if deleted == 0:
            alarm_one.config(text="")
        else:
            alarm_two.config(text="")

    for alarm in alarms:
        if alarm is None:
            continue
        if alarms.index(alarm) == 0:
            alarm_display = f"Alarm 1: {alarm['time']}{', LED On' if alarm['led'] else ''} {', Sound On' if alarm['buzzer'] else ''}"
            alarm_one.config(text=f"Alarm 1 Message: \n {alarm['message']}")
        elif alarms.index(alarm) == 1:
            alarm_display = f"Alarm 2: {alarm['time']}{', LED On' if alarm['led'] else ''} {', Sound On' if alarm['buzzer'] else ''}"
            alarm_two.config(text=f"Alarm 2 Message: \n {alarm['message']}")
        alarm_listbox.insert(tk.END, alarm_display)


def start_optical_communication():
    global flashing_active
    flashing_active = True
    flash_status_label.config(text="Status: Active")
    binary_data = encode_date_time()
    flash_sequence(binary_data)


def stop_optical_communication():
    global flashing_active
    flashing_active = False
    flash_status_label.config(text="Status: Inactive")
    flash_label.config(bg='black')


def encode_date_time():
    """Encode the time and date into a binary string."""
    try:

        time_string = flash_time_entry.get()
        if time_string == "":
            time_string = datetime.now().strftime("%H:%M %d/%m/%Y")

        date_time = time.strptime(time_string, "%H:%M %d/%m/%Y")
        binary_time = f'{date_time.tm_hour:05b}{date_time.tm_min:06b}'
        binary_date = f'{date_time.tm_mday:05b}{date_time.tm_mon:04b}{date_time.tm_year:12b}'
        return f'{binary_time}{binary_date}'
    except ValueError:
        flash_status_label.config(text="Invalid date/time format!")
        return ""


def flash_sequence(binary_data):
    if not binary_data:
        return

    sequence = ['red'] + ['white' if bit ==
                          '1' else 'black' for bit in binary_data]
    while flashing_active:
        for color in sequence:
            if not flashing_active:
                flash_status_label.config(text="Status: Inactive")
                break
            flash_label.config(bg=color)
            root.update()
            time.sleep(0.75)


def write_configuration_letter():
    char = write_config_text.get()
    if not char:
        messagebox.showerror("Error", "No character entered to send.")
        return

    try:
        if ser and ser.is_open:
            ser.write(char.encode('utf-8'))
            # Read the response (ensure the device is sending a response)
            received = ser.readline().decode('utf-8').strip()
            print("Received:", received)
            # Display the received response
            recieve_config_text.config(text=f"Received: {received}")
        else:
            messagebox.showerror("Error", "Not connected to any port")
    except Exception as e:
        messagebox.showerror("Error", f"Failed to write/read serial data: {e}")
        print(f"Error during serial communication: {e}")


def write_configuration():
    if not ser or not ser.is_open:
        messagebox.showerror("Error", "Serial port not connected.")
        return

    # time and format
    current_time_string = start_time.strftime("%H:%M %d/%m/%Y")
    time_format = time_format_var.get()

    # alarm
    alarm_data = []
    for alarm in alarms:
        if alarm:
            alarm_info = f"{alarm['time']},{alarm['message']},{'1' if alarm['led'] else '0'},{'1' if alarm['buzzer'] else '0'}"
            alarm_data.append(alarm_info)
        else:
            alarm_data.append("No")

    if len(alarm_data) == 0:
        alarm_data = ["No", "No"]
    elif len(alarm_data) == 1:
        alarm_data.append("No")

    # weather
    update_weather_data()
    weather_info = ""
    for i in range(7):
        today = (start_time + timedelta(days=i)).strftime("%Y-%m-%d")
        today_parts = today.split('-')
        day = f"{today_parts[0]}-{int(today_parts[1])}-{int(today_parts[2])}"
        weather_day = weather_data['weather'].get(day)

        if weather_day:
            temperature = convert_temperature(weather_day["temperature"])
            humidity = weather_day["humidity"]
            forecast = weather_day["forecast"]
            day_info = f"{day},{forecast.capitalize()},{temperature:.0f},{humidity:.0f}"
        else:
            # If no weather data for this day, indicate "No data"
            day_info = f"{day},No,0,0"

        # Append this day's info to the overall weather info string
        weather_info += day_info + ";"

    # Remove the trailing semicolon
    weather_info = weather_info.rstrip(';')

    # connection status
    connection_status = connection_status_label.cget("text")

    # Format the configuration data
    if alarm_data[0] == "No":
        alarm_data[0] = "No,0,0,0"
    if alarm_data[1] == "No":
        alarm_data[1] = "No,0,0,0"

    configuration_data = (
        f"SEND:{current_time_string},{time_format},{alarm_data[0]},{alarm_data[1]},{temperature_unit},{weather_info}\n"
    )
    try:
        print(configuration_data)
        ser.write(configuration_data.encode('utf-8'))
        ser.flush()
        messagebox.showinfo("Success", "Configuration sent successfully.")
        print(f"Sent configuration data:\n{configuration_data}")
    except Exception as e:
        messagebox.showerror(
            "Error", f"Failed to send configuration data: {e}")
        print(f"Error during serial communication: {e}")


def read_configuration():
    if not ser or not ser.is_open:
        messagebox.showerror("Error", "Serial port not connected.")
        return

    try:
        # Send a request for data to the AVR
        get_data_msg = "GETDATA\n"
        ser.write(get_data_msg.encode('utf-8'))
        ser.flush()
        print("Sent request to AVR for data.")
    except Exception as e:
        messagebox.showerror("Error", f"Failed to send request: {e}")
        print(f"Error during serial communication: {e}")

    try:
        received = ser.readline().decode('utf-8').strip()
        print(f"Received configuration data:\n{received}")

        # Parse the received data
        recieve_config_text.config(text=f"Received: {received}")
        line = received.split(",")

        current_time = line[0]
        time_format = line[1]

        # Set time
        try:
            time_format_var.set("24-hour")
            entered_time = datetime.strptime(current_time, '%H:%M %d/%m/%Y')
            current_time_display.config(text=current_time, font=("Arial", 16))
            global start_time
            start_time = entered_time
            time_format_var.set(time_format)
            display_time()
        except ValueError:
            messagebox.showerror("Error", "Invalid time format received.")

        # Update alarms
        # alarm1, alarm1_led, alarm1_buzzer, alarm1_msg, alarm2, alarm2_led, alarm2_buzzer, alarm2_msg
        alarms.clear()  # Clear existing alarms
        if line[4] == "0":
            line[4] = False
        if line[5] == "0":
            line[5] = False
        if line[6] == "0":
            line[6] = False
        if line[7] == "0":
            line[7] = False

        if line[2] != "No":
            alarm1 = {
                "time": line[2],
                "message": line[9],
                "led": line[4],
                "buzzer": line[5]
            }
            alarms.append(alarm1)
        if line[3] != "No":
            alarm2 = {
                "time": line[3],
                "message": line[10],
                "led": line[6],
                "buzzer": line[7]
            }
            alarms.append(alarm2)

        update_alarm_listbox(deleted="both")

        global temperature_unit
        temperature_unit = line[8]
        weather_format_var.set(temperature_unit)
        display_weather_data()

        # line[11] is the weather data
        given_forecast_listbox.delete(0, tk.END)
        for i in range(7):
            forecast_data = line[11 + i*4], line[12 +
                                                 i*4], line[13 + i*4], line[14 + i*4]
            given_forecast_listbox.insert(tk.END, f"{forecast_data}")

    except Exception as e:
        messagebox.showerror(
            "Error", f"Failed to read configuration data: {e}")
        print(f"Error during serial communication: {e}")


def show_ports():
    ports = list(serial.tools.list_ports.comports())
    ports_list = [p.device for p in ports]
    available_ports_listbox['values'] = ports_list
    # return ports_list


def disconnect_port():
    global ser
    if ser and ser.is_open:
        ser.close()
        connection_status_label.config(text="Connection Status: Disconnected")
    else:
        messagebox.showerror("Error", "Serial port not connected.")


def connect_port():
    print("Connecting to port:", available_ports_listbox.get())
    port = available_ports_listbox.get()
    baudrate = 9600  # 115200
    try:
        global ser
        ser = serial.Serial(port, baudrate, timeout=0.5)
        if ser.is_open:
            connection_status_label.config(
                text=f"Connection Status: Connected to {port}")
            try:
                connected_msg = "Connection Status: Connected\n"
                ser.write_timeout = 1
                ser.write(connected_msg.encode('utf-8'))
                ser.flush()

            except serial.SerialTimeoutException:
                ser.close()
                messagebox.showerror(
                    "Serial Error", "Timeout while sending data. The port may not be writable.")
                connection_status_label.config(
                    text="Connection Status: Disconnected")

            except serial.SerialException as e:
                ser.close()
                messagebox.showerror(
                    "Serial Error", f"Failed to send data: {e}")
                connection_status_label.config(
                    text="Connection Status: Disconnected")
        else:
            messagebox.showerror("Error", "Failed to open the serial port.")

    except serial.SerialException as e:
        messagebox.showerror(
            "Serial Error", "Timeout while sending data. The port may not be writable.")
        connection_status_label.config(text="Connection Status: Disconnected")


# Create main window
root = tk.Tk()
root.title("E-Paper Display Configurator")

# Styling
style = ttk.Style()
style.configure("TLabel", font=("Arial", 10))
style.configure("TButton", font=("Arial", 10))
style.configure("TFrame", padding=10)
style.configure("TLabelframe", padding=10, font=("Arial", 10, "bold"))

# Time and Date Setting
time_frame = ttk.Labelframe(root, text="Time and Date Setting")
time_frame.pack(padx=10, pady=5, fill="x")

current_time_label = ttk.Label(time_frame, text="Current Time:")
current_time_label.grid(row=0, column=0, sticky="w")

# Adjust the font size to be smaller for a better fit
current_time_display = ttk.Label(time_frame, text=datetime.now().strftime(
    "%I:%M %p") + " " + datetime.now().strftime("%d/%m/%Y"), font=("Arial", 16))
current_time_display.grid(row=0, column=1, sticky="w", columnspan=3)

time_format_label = ttk.Label(time_frame, text="Time Format:")
time_format_label.grid(row=1, column=0, sticky="w")
time_format_var = tk.StringVar(value="12-hour")
Radiobutton_12h = ttk.Radiobutton(
    time_frame, text="12-hour", variable=time_format_var, value="12-hour", command=display_time)
Radiobutton_12h.grid(row=1, column=1, sticky="w")
Radiobutton_24h = ttk.Radiobutton(
    time_frame, text="24-hour", variable=time_format_var, value="24-hour", command=display_time)
Radiobutton_24h.grid(row=1, column=2, sticky="w")

manual_time_label = ttk.Label(time_frame, text="Manual Time Entry:")
manual_time_label.grid(row=2, column=0, sticky="w")
manual_time_entry = ttk.Entry(time_frame)
manual_time_entry.grid(row=2, column=1, sticky="w")
set_time_button = ttk.Button(time_frame, text="Set Time", command=set_time)
set_time_button.grid(row=2, column=2, sticky="w")

set_system_time_button = ttk.Button(
    time_frame, text="Set to System Time", command=set_system_time)
set_system_time_button.grid(row=2, column=3, sticky="w")


normal_mode_button = ttk.Button(
    time_frame, text="Normal Mode", command=set_normal_mode)
normal_mode_button.grid(row=3, column=0, sticky="w")
accelerated_mode1_button = ttk.Button(
    time_frame, text="1 Sec -> 15 Sec", command=set_accelerated_mode1)
accelerated_mode1_button.grid(row=3, column=1, sticky="w")
accelerated_mode2_button = ttk.Button(
    time_frame, text="1 Sec -> 1 hr", command=set_accelerated_mode2)
accelerated_mode2_button.grid(row=3, column=2, sticky="w")

# Alarm Configuration
alarm_frame = ttk.Labelframe(root, text="Alarm Configuration")
alarm_frame.pack(padx=10, pady=5, fill="x")

alarm_listbox = tk.Listbox(alarm_frame, height=5, width=40)
alarm_listbox.grid(row=0, column=3, columnspan=3, sticky="we")

alarm_one = ttk.Label(alarm_frame, text="", wraplength=250)
alarm_one.grid(row=0, column=6, sticky="we")

alarm_two = ttk.Label(alarm_frame, text="", wraplength=250)
alarm_two.grid(row=0, column=8, sticky="we")

time_label = ttk.Label(alarm_frame, text="Time (HH:MM DD/MM/YYYY) :")
time_label.grid(row=1, column=0, sticky="w")
time_entry = ttk.Entry(alarm_frame)
time_entry.grid(row=1, column=1, sticky="w")

message_label = ttk.Label(alarm_frame, text="Message:")
message_label.grid(row=0, column=0, sticky="w")
message_entry = tk.Text(alarm_frame, width=25, height=5)
message_entry.grid(row=0, column=1, sticky="w")

led_var = tk.BooleanVar()
led_checkbutton = ttk.Checkbutton(
    alarm_frame, text="LED Blink", variable=led_var)
led_checkbutton.grid(row=3, column=0, sticky="w")

buzzer_var = tk.BooleanVar()
buzzer_checkbutton = ttk.Checkbutton(
    alarm_frame, text="Buzzer", variable=buzzer_var)
buzzer_checkbutton.grid(row=3, column=1, sticky="w")

add_update_alarm_button = ttk.Button(
    alarm_frame, text="Add/Update Alarm", command=add_update_alarm)
add_update_alarm_button.grid(row=4, column=0, sticky="w")
delete_alarm_button = ttk.Button(
    alarm_frame, text="Delete Alarm", command=delete_alarm)
delete_alarm_button.grid(row=4, column=1, sticky="w")

# Weather Data Management
weather_frame = ttk.Labelframe(root, text="Weather Data Management")
weather_frame.pack(padx=10, pady=5, fill="x")

current_weather_label = ttk.Label(weather_frame, text="Current Weather: N/A")
current_weather_label.grid(row=0, column=0, columnspan=3, sticky="w")

forecast_label = ttk.Label(weather_frame, text="Forecast:")
forecast_label.grid(row=1, column=0, sticky="w")
forecast_listbox = tk.Listbox(weather_frame, height=8, width=40)
forecast_listbox.grid(row=2, column=0, columnspan=3, sticky="we")

given_forecast_label = ttk.Label(weather_frame, text="Imported Forcast:")
given_forecast_label.grid(row=1, column=3, sticky="w")
given_forecast_listbox = tk.Listbox(weather_frame, height=8, width=40)
given_forecast_listbox.grid(row=2, column=3, columnspan=3, sticky="we")

update_weather_button = ttk.Button(
    weather_frame, text="Update Weather Data", command=update_weather_data)
update_weather_button.grid(row=4, column=0, sticky="w")

weather_format_label = ttk.Label(weather_frame, text="Weather Format:")
weather_format_label.grid(row=3, column=0, sticky="w")
weather_format_var = tk.StringVar(value="C")
ttk.Radiobutton(weather_frame, text="C", variable=weather_format_var,
                value="C", command=update_weather_unit).grid(row=3, column=1, sticky="w")
ttk.Radiobutton(weather_frame, text="F", variable=weather_format_var,
                value="F", command=update_weather_unit).grid(row=3, column=2, sticky="w")

# Flashing display for optical communication
flash_frame = ttk.Labelframe(root, text="Optical Communication Display")
flash_frame.pack(padx=10, pady=5, fill="x")

flash_status_label = ttk.Label(flash_frame, text="Status: Inactive")
flash_status_label.grid(row=1, column=0, sticky="w")

flash_time_label = ttk.Label(flash_frame, text="Time (HH:MM DD/MM/YYYY):")
flash_time_label.grid(row=0, column=0, sticky="w")
flash_time_entry = ttk.Entry(flash_frame)
flash_time_entry.grid(row=0, column=1, sticky="w")

flash_label = tk.Label(flash_frame, width=40, height=5, bg='black')
flash_label.grid(row=0, column=2, columnspan=2, rowspan=3, sticky="we")

start_flashing_button = ttk.Button(
    flash_frame, text="Start Optical Communication", command=start_optical_communication)
start_flashing_button.grid(row=2, column=0, padx=5)

stop_flashing_button = ttk.Button(
    flash_frame, text="Stop", command=stop_optical_communication)
stop_flashing_button.grid(row=2, column=1, padx=5)

# Status Indicators
status_frame = ttk.Labelframe(root, text="Status")
status_frame.pack(padx=10, pady=5, fill="x")

connection_status_label = ttk.Label(
    status_frame, text="Connection Status: Disconnected")
connection_status_label.grid(row=0, column=0, sticky="w")


sleep_mode_label = ttk.Label(status_frame, text="Sleep Mode: Inactive")
sleep_mode_label.grid(row=2, column=0, sticky="w")

available_ports_label = ttk.Label(status_frame, text="Available Ports:")
available_ports_label.grid(row=0, column=2, sticky="w", padx=20)
available_ports_listbox = ttk.Combobox(status_frame, postcommand=show_ports)
available_ports_listbox.grid(row=0, column=3, sticky="w")

connect_button = ttk.Button(status_frame, text="Connect", command=connect_port)
connect_button.grid(row=0, column=4, sticky="w")

disconnect_button = ttk.Button(
    status_frame, text="Disconnect", command=disconnect_port)
disconnect_button.grid(row=0, column=5, sticky="w")

# Configuration Controls
config_frame = ttk.Frame(root)
config_frame.pack(padx=10, pady=5, fill="x")

write_config_button = ttk.Button(
    config_frame, text="Write/Send", command=write_configuration)
write_config_button.grid(row=0, column=1, padx=5)
# write_config_text = ttk.Entry(config_frame)
# write_config_text.grid(row=0, column=2, padx=5)
recieve_config_button = ttk.Button(
    config_frame, text="Receive", command=read_configuration)
recieve_config_button.grid(row=0, column=3, padx=5)
recieve_config_text = ttk.Label(config_frame, text="")
recieve_config_text.grid(row=0, column=4, padx=5)

# Start the main loop and display the current system time immediately
set_system_time()  # Initializes the time display
start_time = start_time - timedelta(seconds=60)
increment_time()  # Start the continuous time display
root.mainloop()
