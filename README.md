# Insomnia Tracker
Bedside button that you push when you can't sleep. Timestamps of button presses are sent to Adafruit.io via MQTT, and the data is downloaded and processed using R to send a daily sleep summary email.
If you can't sleep, you might as well collect data!

The hardware used is virtually identical to the Long Distance Relationship Cube project. The Arduino code is also almost identical, with the RGB LED functionality disabled.
The R script scrapes the logged MQTT data on Adafruit.io using the Adafruit.io V2 API and keeps the data in a locally stored spreadsheet. Summary statistics and a chart are assembled and emailed.
The R script runs automatically every day using the Windows Task Scheduler (batch script coming soon). 
