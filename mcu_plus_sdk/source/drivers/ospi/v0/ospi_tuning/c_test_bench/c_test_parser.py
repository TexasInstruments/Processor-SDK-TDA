import csv
import re

# Define the output CSV file
output_csv = 'output_new_latest.csv'

# Define the field names for the CSV file
field_names = ['Iteration', 'isValid','Otp1Rx', 'Otp1Tx', 'Otp1Rd', 'Otp1Time', 
               'Otp2Rx', 'Otp2Tx', 'Otp2Rd', 'Otp2Time Radius 5', 'Otp2Time Radius 10', 'Otp1Result', 'Otp2Result']

# Open the output CSV file for writing
with open(output_csv, 'w', newline='') as csvfile:
    writer = csv.DictWriter(csvfile, fieldnames=field_names)

    # Write the header row
    writer.writeheader()

    # Open the output.txt file for reading
    with open('output.txt', 'r') as txtfile:
        for line in txtfile:
            # Split the line into parts
            parts = line.strip().split()

            # Check if the line has the correct format
            if len(parts) >= 17:
                # Extract the values
                Iteration = parts[1].replace('Iteration:', '')
                isValid = parts[3].replace('TestCase:', '')
                otp1_rx = parts[7].replace('rx =', '').replace(',', '')
                otp1_tx = parts[10].replace('tx =', '').replace(',', '')
                otp1_rd = parts[13].replace('rd =', '').replace(',', '')
                otp1_time = parts[15].replace('time:', '').replace('us', '')
                otp2_rx = parts[19].replace('rx =', '').replace(',', '')
                otp2_tx = parts[22].replace('tx =', '').replace(',', '')
                otp2_rd = parts[25].replace('rd =', '').replace(',', '')
                otp2_time_1 = parts[29].replace('time:', '').replace('us', '').replace(',', '')
                otp2_time_2 = parts[33].replace('time:', '').replace('us', '')
                otp1_result = 'FAIL' if 'OTP1 result: FAIL' in line else 'PASS'
                otp2_result = 'FAIL' if 'OTP2 result: FAIL' in line else 'PASS'

                # Write the values to the CSV file
                writer.writerow({
                    'Iteration': Iteration,
                    'isValid': isValid,
                    'Otp1Rx': otp1_rx,
                    'Otp1Tx': otp1_tx,
                    'Otp1Rd': otp1_rd,
                    'Otp1Time': otp1_time,
                    'Otp2Rx': otp2_rx,
                    'Otp2Tx': otp2_tx,
                    'Otp2Rd': otp2_rd,
                    'Otp2Time Radius 5': otp2_time_1,
                    'Otp2Time Radius 10': otp2_time_2,
                    'Otp1Result': otp1_result,
                    'Otp2Result': otp2_result
                })

print(f"Output parsed and written to {output_csv}")