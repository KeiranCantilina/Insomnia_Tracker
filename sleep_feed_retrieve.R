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
old_data <- import(old_data_path)

## Remove duplicate entries to avoid fencepost errors (based on duplicate IDs)
if (old_data$id[1] == data_collated$id[length(data_collated$id)]){
  data_collated <- data_collated[1:(length(data_collated$id)-1),]
}

## append cdataframe on to spreadsheet
combined_data <- rbind(data_collated,old_data)


## export/save
write.csv(combined_data, file=old_data_path)

## Capture latest timestamp
new_timestamp <- data_raw$created_at[1] ## New entries are at the top of the list, not the bottom

## Save timestamp to text file
write_file(new_timestamp,timestamp_file_path, append=FALSE)

## calculate stats about sleep period

number_of_awakes <- length(data_collated$id)
time_between_wakes <- c()
  ## Average time between wakes
  ## Median time between wakes

## Generate chart

## Compose and send email
