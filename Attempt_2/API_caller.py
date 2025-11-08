#!/usr/bin/env python3

"""
A simple API calling script for Vim.

Takes two arguments:
1. A URL template (e.g., "https://api.example.com/search?q=%s")
2. A parameter string to be URL-encoded and inserted into the template.

Prints the API's response text to standard output.
"""

import sys
import urllib.request
import urllib.parse

def fetch_api_data(url_template, parameter):
    """
    Fetches data from a URL template with an encoded parameter.
    """
    if len(sys.argv) != 3:
        print("Error: Invalid arguments.", file=sys.stderr)
        print("Usage: python api_caller.py <url_template> <parameter>", file=sys.stderr)
        sys.exit(1)

    try:
        # URL-encode the parameter (e.g., "my search" -> "my+search")
        encoded_parameter = urllib.parse.quote_plus(parameter)
        
        # Format the final URL by replacing '%s' with the encoded parameter
        final_url = url_template % encoded_parameter
        
        # Open the URL and read the response
        with urllib.request.urlopen(final_url) as response:
            # Decode the response as UTF-8 text
            return response.read().decode('utf-8')
            
    except Exception as e:
        # Print any errors to stderr so Vim can report them
        print(f"Error fetching URL: {e}", file=sys.stderr)
        sys.exit(1)

if __name__ == "__main__":
    url_template = sys.argv[1]
    parameter = sys.argv[2]
    
    output = fetch_api_data(url_template, parameter)
    if output:
        print(output)
