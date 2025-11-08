#!/usr/bin/env python3

"""
A simple API calling script for Vim,

Takes two arguments:
1. The API endpoint URL (e.g., "https://generativelanguage.googleapis.com/v1beta/models/gemini-pro:generateContent")
2. The user's prompt string.

Reads the API key from the 'GEMINI_API_KEY' environment variable.

Prints the API's text response to standard output.
"""

import sys
import os
import json
import urllib.request

def fetch_gemini_data(endpoint_url, prompt):
    """
    Fetches data from the Gemini API using a POST request.
    """
    try:
        # 1. Get API Key from environment variable
        api_key = os.environ.get('GEMINI_API_KEY')
        if not api_key:
            print("Error: GEMINI_API_KEY environment variable not set.", file=sys.stderr)
            sys.exit(1)
            
        # 2. Construct the full URL with the API key
        full_url = f"{endpoint_url}?key={api_key}"
        
        # 3. Construct the Gemini-specific JSON payload
        payload = {
            "contents": [
                {
                    "parts": [
                        {"text": prompt}
                    ]
                }
            ]
        }
        
        # 4. Prepare the POST request
        # We must encode the payload as bytes and set the Content-Type header.
        data = json.dumps(payload).encode('utf-8')
        headers = {'Content-Type': 'application/json'}
        
        req = urllib.request.Request(full_url, data=data, headers=headers, method='POST')
        
        # 5. Make the request and read the response
        with urllib.request.urlopen(req) as response:
            response_text = response.read().decode('utf-8')
            response_json = json.loads(response_text)
            
            # 6. Parse the Gemini-specific response to get the text
            # This navigates the JSON: response.candidates[0].content.parts[0].text
            text_output = response_json['candidates'][0]['content']['parts'][0]['text']
            return text_output
            
    except urllib.error.HTTPError as e:
        # Print HTTP errors (like 400, 403, 500)
        error_body = e.read().decode('utf-8')
        print(f"HTTP Error: {e.code}\n{error_body}", file=sys.stderr)
        sys.exit(1)
    except KeyError:
        # Handle cases where the JSON response is not what we expect
        print(f"Error: Could not parse API response.\n{response_text}", file=sys.stderr)
        sys.exit(1)
    except Exception as e:
        # Catch other errors
        print(f"An unexpected error occurred: {e}", file=sys.stderr)
        sys.exit(1)

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python api_caller_post.py <endpoint_url> <prompt>", file=sys.stderr)
        sys.exit(1)

    endpoint = sys.argv[1]
    prompt_text = sys.argv[2]
    
    output = fetch_gemini_data(endpoint, prompt_text)
    if output:
        print(output)
