#!/usr/bin/env python3

import time

def main():
    # Print the HTTP header and the HTML content
    print("Content-Type: text/html")
    print("Status: 200 OK", end="\r\n\r\n")
    
    # Output HTML with JavaScript to refresh the time every second
    print("""
    <html>
    <head>
        <title>Real-time Clock</title>
        <script type="text/javascript">
            function updateTime() {
                // Create a new Date object
                var now = new Date();
                
                // Get the current time in HH:MM:SS format
                var hours = now.getHours().toString().padStart(2, '0');
                var minutes = now.getMinutes().toString().padStart(2, '0');
                var seconds = now.getSeconds().toString().padStart(2, '0');
                var timeString = hours + ':' + minutes + ':' + seconds;
                
                // Update the content of the timeDisplay element
                document.getElementById('timeDisplay').innerText = timeString;
            }

            // Update time every second
            setInterval(updateTime, 1000);

            // Initial call to display the time immediately on page load
            window.onload = updateTime;
        </script>
    </head>
    <body>
        <h1>Current Time</h1>
        <div id="timeDisplay"></div>
    </body>
    </html>
    """)

if __name__ == "__main__":
    main()
