
import time
import json
import logging
# import subprocess 

import grpc  # type: ignore
import client_pb2 as client_pb2  # type: ignore
import client_pb2_grpc as client_pb2_grpc  # type: ignore

KEYGROUP = "simple"



logging.basicConfig(level=logging.DEBUG, format="%(asctime)s - %(levelname)s - %(filename)s:%(lineno)d - %(message)s")



MOCK_FRED_ADDRESS = 'localhost:9001' 
logging.info(f"put_fn: Connecting to MockFReD at {MOCK_FRED_ADDRESS}")

channel = grpc.insecure_channel(MOCK_FRED_ADDRESS)
stub = client_pb2_grpc.ClientStub(channel)


def put(key: str, value: str) -> None:
    
    
    ur = client_pb2.UpdateRequest(
        keygroup=KEYGROUP,
        id=key,
        data=str(value), 
    )
    logging.debug(f"put_fn: Sending UpdateRequest for key: {key}, value: {value}")
    try:
        response = stub.Update(ur) 
        logging.debug(f"put_fn: Update successful for key '{key}'. Response version: {response.version.version if response and response.version else 'N/A'}")
    except grpc.RpcError as e:
        logging.error(f"put_fn: gRPC error putting value for key '{key}': {e.code()} - {e.details()}")
        raise 


def fn(inp: str) -> str: 
    # input is a little json string
    # {"key": "sensor1", "value": "8.9"} 

    t1 = time.perf_counter()
    logging.info(f"put_fn (FaaS entry): Received request with payload: {inp}")

    try:
        parsed_input = json.loads(inp)
        key = parsed_input["key"]
        
        value = str(parsed_input["value"]) 
    except (json.JSONDecodeError, KeyError) as e:
        logging.error(f"put_fn (FaaS entry): Invalid JSON input or missing key/value: {inp} - Error: {e}")
        
        raise ValueError(f"Invalid input: {inp}. Expected JSON with 'key' and 'value'.")


    logging.debug(f"put_fn (FaaS entry): Received value '{value}' for key '{key}'")

    try:
        put(key, value)
    except Exception as e: 
        logging.error(f"put_fn (FaaS entry): Error putting value for key '{key}': {e}")
        raise 

    time_taken = time.perf_counter() - t1
    logging.info(f"put_fn (FaaS entry): Time taken: {time_taken:.4f}s. Successfully processed put for key '{key}'.")
    return "OK"



if __name__ == '__main__':
    logging.getLogger().setLevel(logging.INFO) 

    

    print("Testing put_fn locally...")
    test_payload_str = '{"key": "testkey_put_local", "value": "local_put_value_123"}'
    print(f"Sending payload: {test_payload_str}")
    try:
        result = fn(test_payload_str)
        print(f"Result from put_fn: {result}")
        
       

    except Exception as e:
        print(f"Error during local test of put_fn: {e}")