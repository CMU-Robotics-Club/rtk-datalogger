PARSE_CSV_HEADER = ["timestamp",
                    "latitude",
                    "longitude",
                    "altitude",
                    "fix-type",
                    "satellites",
                    "sdn",
                    "sde",
                    "sdu",
                    "sdne",
                    "sdeu",
                    "sdun",
                    "age",
                    "ratio"
]

CALC_CSV_HEADER = [
    "timestamp", # a datetime object (as a string)
    "timestamp_str",
    "fix_type",
    "latitude",
    "longitude",
    "ground_speed_raw", # m/s
    "ground_speed_processed", # m/s, TODO: should be smoothed
    "acceleration",
    "altitude", # meters
    "horizontal_accuracy",
    "vertical_accuracy",
    "speed_accuracy"
 ]
