#!/usr/bin/python3

import cgi
import os
import datetime

# Get form data
form = cgi.FieldStorage()

# Get the values of the two integers and the operation
num1 = form.getvalue("num1")
num2 = form.getvalue("num2")
operation = form.getvalue("operation")

# Initialize result variable
result = None

# Convert numbers to integers and perform calculation
if num1 is not None and num2 is not None:
    try:
        num1 = int(num1)
        num2 = int(num2)
        if operation == "add":
            result = num1 + num2
        elif operation == "subtract":
            result = num1 - num2
        elif operation == "multiply":
            result = num1 * num2
        elif operation == "divide":
            if num2 != 0:
                result = num1 / num2
            else:
                result = "Error: Division by zero"
        else:
            result = "Error: Invalid operation"
    except ValueError:
        result = "Error: Invalid input"

# Print the HTTP headers and the HTML content
print("Content-type: text/html")
print("Status: 200 OK\r\n\r\n")

print("<html>")
print("<head>")
print("<title>Simple Calculator</title>")
print("</head>")
print("<body>")
print("<h1>Simple Calculator</h1>")

# Display the current time
current_time = datetime.datetime.now().strftime("%H:%M:%S")
print(f"<h2>Current Time: {current_time}</h2>")

# Display the form
print("""
<form method="post" action="/cgi-bin/cal.py">
    <label for="num1">First Number:</label>
    <input type="text" id="num1" name="num1"><br><br>
    <label for="num2">Second Number:</label>
    <input type="text" id="num2" name="num2"><br><br>
    <label for="operation">Operation:</label>
    <select id="operation" name="operation">
        <option value="add">Add</option>
        <option value="subtract">Subtract</option>
        <option value="multiply">Multiply</option>
        <option value="divide">Divide</option>
    </select><br><br>
    <input type="submit" value="Calculate">
</form>
""")

# Display the result if available
if result is not None:
    print(f"<h2>Result: {result}</h2>")

print("</body>")
print("</html>")
