#!/usr/bin/python3

import datetime
import cgi

chunk_size = 5

# Headers
print("Content-type: text/html")
print("Status: 200 OK")
print("Transfer-Encoding: chunked\r\n\r\n", end='')

# Body
fmt = "<h1>  %H:%M:%S </h1>"
body = f"<html><head>{datetime.datetime.strftime(datetime.datetime.now(), fmt)}</head></html>"

for startIndex in range(0, len(body), chunk_size):
	endIndex = (startIndex + chunk_size) if (startIndex + chunk_size) <= len(body) else len(body)
	chunk = body[startIndex:endIndex]
	print(endIndex - startIndex)
	print(chunk)
print(0)
