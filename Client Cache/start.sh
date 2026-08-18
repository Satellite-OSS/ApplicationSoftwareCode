#! /bin/bash
echo "Starting mock FReD gRPC server..."
python mock_fred_server.py &
PID_FRED=$! 


echo "Starting mock FaaS HTTP server..."
python mock_faas_http_server.py &
PID_FAAS=$! 


echo "Waiting for servers to initialize (5 seconds)..."
sleep 5


echo "Running client..."
/binary/client client1
CLIENT_EXIT_CODE=$? 

echo "Client finished with exit code $CLIENT_EXIT_CODE."


echo "Stopping background servers..."
kill $PID_FRED
kill $PID_FAAS


wait $PID_FRED
wait $PID_FAAS
echo "Servers stopped."


exit $CLIENT_EXIT_CODE
