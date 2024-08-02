#!/usr/bin/env python3

import cgi
import cgitb
import os
import sys

# Enable error reporting
cgitb.enable()

# Print HTTP header
print("Content-Type: text/html\n")

# Create instance of FieldStorage
form = cgi.FieldStorage()

print("This is an error message.", file=sys.stderr)

# Get data from fields
username = form.getvalue('username')
file_item = form['file']

print("This is an error message.", file=sys.stderr)

# Start HTML response
print("<html><body>")
print("<h1>CGI POST Example</h1>")

# Process username
if username:
    print(f"<p>Username: {username}</p>")
else:
    print("<p>No username provided.</p>")

print("This is an error message.", file=sys.stderr)

# Process file upload
if file_item.filename:
    # Construct file path
    file_path = os.path.join("/tmp", os.path.basename(file_item.filename))
    # Save file to server
    with open(file_path, 'wb') as file:
        file.write(file_item.file.read())
    print(f"<p>File {file_item.filename} uploaded successfully to {file_path}.</p>")
else:
    print("<p>No file uploaded.</p>")

# End HTML response
print("</body></html>")
