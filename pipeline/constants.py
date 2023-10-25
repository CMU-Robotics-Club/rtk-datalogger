PARSE_CSV_HEADER = ["timestamp", # TODO change this to "timestamp_str"
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
    "timestamp", # UTC time (float)
    "timestamp_str",
    "fix_type",
    "latitude",
    "longitude",
    "altitude", # meters
    "velocity", # 2d vector
    "speed", # scalar (abs val of vector)
    "acceleration_vector",
    "acceleration_magnitude",
    "horizontal_accuracy",
    "vertical_accuracy",
    "speed_accuracy"
 ]
