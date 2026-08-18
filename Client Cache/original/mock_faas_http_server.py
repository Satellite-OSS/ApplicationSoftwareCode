

from flask import Flask, request, Response
import logging
import json
import sys 

logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')


faas_functions_imported_successfully = False
try:
    import get_fn
    import put_fn
    faas_functions_imported_successfully = True
    logging.info("Successfully imported get_fn and put_fn modules.")
except ImportError as e:
    logging.error(f"CRITICAL: Failed to import FaaS function modules (get_fn, put_fn): {e}")
    logging.error("Please ensure 'get_fn.py', 'put_fn.py', 'client_pb2.py', and 'client_pb2_grpc.py' are in the same directory as this script, or accessible via PYTHONPATH.")
    

app = Flask(__name__)

@app.route('/put', methods=['POST'])
def handle_put():
    if not faas_functions_imported_successfully:
        logging.error("MockFaaS HTTP: 'put_fn' module was not imported. Cannot process /put request.")
        return Response("Internal Server Error: FaaS module not loaded.", status=500)
    try:
        json_data_string = request.data.decode('utf-8')
        logging.info(f"MockFaaS HTTP: Received /put request with data: {json_data_string}")
        result = put_fn.fn(json_data_string)
        logging.info(f"MockFaaS HTTP: /put processed, result: {result}")
        return Response(result, status=200, mimetype='text/plain')
    except Exception as e:
       
        logging.error(f"MockFaaS HTTP: Error processing /put: {e}", exc_info=True) 
        return Response(f"Error processing /put: {str(e)}", status=500)

@app.route('/get', methods=['POST'])
def handle_get():
    if not faas_functions_imported_successfully:
        logging.error("MockFaaS HTTP: 'get_fn' module was not imported. Cannot process /get request.")
        return Response("Internal Server Error: FaaS module not loaded.", status=500)
    try:
        key_string = request.data.decode('utf-8')
        logging.info(f"MockFaaS HTTP: Received /get request for key: {key_string}")
        value = get_fn.fn(key_string)
        logging.info(f"MockFaaS HTTP: /get processed, value for key '{key_string}': {value}")
        return Response(value, status=200, mimetype='text/plain')
    except Exception as e:
        logging.error(f"MockFaaS HTTP: Error processing /get for key '{key_string}': {e}", exc_info=True)
        
        if hasattr(e, 'code') and callable(e.code) and e.code() == grpc.StatusCode.NOT_FOUND:
             logging.warning(f"MockFaaS HTTP: Key '{key_string}' not found by FaaS function.")
             return Response(f"Key '{key_string}' not found", status=404)
        return Response(f"Error processing /get: {str(e)}", status=500)

if __name__ == '__main__':
    if not faas_functions_imported_successfully:
        logging.error("MockFaaS HTTP server cannot start because FaaS function modules (get_fn, put_fn) failed to import.")
    else:
        logging.info("MockFaaS HTTP server starting on port 8000")
        app.run(host='0.0.0.0', port=8000, debug=False)