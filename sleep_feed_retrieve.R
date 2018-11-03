## Script for retrieving raw data from an MQTT feed on Adafruit.io

## Script should run automatically at some point away from sleeping times, but relatively soon after wakeup time (11AM ish)

library(curl)
library(RCurl)
library(rio)
library(stringr)
library(rjson)
library(readr)

## Check for valid internet connection
if (url.exists("www.google.com")==TRUE) {
  out <- TRUE
} else {
  out <- FALSE
}

if(out==FALSE){
  quit()
}

## Read timestamp text file
timestamp <- NA
timestamp_file_path <- "C://Users//Keiran//Documents//awake_counter_timestamp_file.txt"
timestamp_file <- read_file(timestamp_file_path)
timestamp <- paste(timestamp_file, "%22", sep="")

## If timestamp file is empty, do not add time query to URL
start_time_query <- NA
if (is.na(timestamp) == FALSE){
  start_time_query <- "?start_time=%22"
}


## Retrieve data from feed
URL <- paste("https://io.adafruit.com/api/v2/Keirancantilina/feeds/keiran-block/data", na.omit(start_time_query), na.omit(timestamp), ".json", sep="")
data_raw <-import(URL, format="json")


## Discard unused columns
data_collated <- data_raw[c(1,5,10)]


## Add data to spreadsheet
old_data_path <- "C://Users//Keiran//Desktop//RLO sleep log//RLO_sleep_log.csv"
old_data <- read.csv(old_data_path)

## Remove duplicate entries to avoid fencepost errors (based on duplicate IDs)
if (old_data$id[1] == data_collated$id[length(data_collated$id)]){
  data_collated[length(data_collated$id),] <- NA
  data_collated <- data_collated[1:(length(data_collated$id)-1),]
}

## append cdataframe on to spreadsheet
combined_data <- na.omit(rbind(data_collated,old_data))


## export/save
write.csv(combined_data, file=old_data_path, row.names=FALSE)

## Capture latest timestamp
new_timestamp <- data_raw$created_at[1] ## New entries are at the top of the list, not the bottom

## Save timestamp to text file
write_file(new_timestamp,timestamp_file_path, append=FALSE)

## calculate stats about sleep period
time_between_wakes <- c()
number_of_wakes <- length(na.omit(data_collated$id))

for (i in 2:number_of_wakes){
  time_between_wakes[i-1] <- data_collated$created_epoch[i-1]-data_collated$created_epoch[i]
}

average_today <- mean(time_between_wakes)
median_today <- median(time_between_wakes)

if (number_of_wakes==0){
  average_today <- "No wakes last night!"
  median_today <- "No wakes last night!"
}
  

## Generate chart

## first coerce all dates to common format
list_of_dates <- as.Date(as.POSIXct(as.numeric(combined_data$created_epoch), origin="1970-01-01"))
histogram <- hist(list_of_dates, breaks="days")

## Compose and send email
## all of the paste functions