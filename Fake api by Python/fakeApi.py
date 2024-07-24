import requests
import json
import random
import time

# Replace with your device access token
ACCESS_TOKEN = 'rpX1dv3tqbPZ8gM0zs2Z'

# Replace with your ThingsBoard server URL
THINGSBOARD_URL = 'http://localhost:8080'

# Endpoint URL for telemetry
url = f'{THINGSBOARD_URL}/api/v1/{ACCESS_TOKEN}/telemetry'

# Function to generate random GPS data
def generate_random_gps():
    # Hanoi coordinates (approximately)
    lat = 21.0285 + random.uniform(-0.0005, 0.0005)
    lon = 105.8542 + random.uniform(-0.0005, 0.0005)
    sp = 40 + random.uniform(0, 20)
    timestamp = int(time.time()) * 1000  # Current timestamp in milliseconds
    return {
        'status': 'On route',
        'speed' : sp,
        'fuel' : 80,
        'latitude': lat,
        'longitude': lon,
        'timestamp': timestamp
    }

# Generate and send random GPS data
while True:
    gps_data = generate_random_gps()
    
    # Convert the data to JSON format
    json_data = json.dumps(gps_data)
    
    # Set the headers
    headers = {
        'Content-Type': 'application/json'
    }
    
    # Send the request
    response = requests.post(url, headers=headers, data=json_data)
    
    # Check the response
    if response.status_code == 200:
        print(f'GPS data sent successfully: {gps_data}')
    else:
        print('Failed to send GPS data')
        print('Response code:', response.status_code)
        print('Response body:', response.text)
    
    # Wait for 2 seconds before sending the next data (for simulation)
    time.sleep(2)