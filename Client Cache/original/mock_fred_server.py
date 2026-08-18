
import grpc
from concurrent import futures
import time
import logging


import client_pb2 
import client_pb2_grpc

logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')

_ONE_DAY_IN_SECONDS = 60 * 60 * 24


DATA_STORE = {
    "simple": {} 
}

VERSION_COUNTER = {
    "simple": {} # {'item_id': version_number}
}

class MockFredServicer(client_pb2_grpc.ClientServicer):
    def Read(self, request, context):
        logging.info(f"MockFReD: Received Read request for keygroup='{request.keygroup}', id='{request.id}'")
        keygroup = request.keygroup
        item_id = request.id
        
        if keygroup in DATA_STORE and item_id in DATA_STORE[keygroup]:
            value = DATA_STORE[keygroup][item_id]
            item_version_map = {}
            if item_id in VERSION_COUNTER.get(keygroup, {}):
                 
                item_version_map[item_id] = VERSION_COUNTER[keygroup][item_id]

            item = client_pb2.Item(
                id=item_id, 
                val=value, 
                version=client_pb2.Version(version=item_version_map)
            )
            logging.info(f"MockFReD: Found item. Returning value='{value}'")
            return client_pb2.ReadResponse(data=[item])
        else:
            logging.warning(f"MockFReD: Item not found for keygroup='{keygroup}', id='{item_id}'")
            context.set_code(grpc.StatusCode.NOT_FOUND)
            context.set_details(f"Item with id '{item_id}' not found in keygroup '{keygroup}'")
            return client_pb2.ReadResponse()

    def Update(self, request, context):
        logging.info(f"MockFReD: Received Update request for keygroup='{request.keygroup}', id='{request.id}', data='{request.data}'")
        keygroup = request.keygroup
        item_id = request.id
        data = request.data

        if keygroup not in DATA_STORE:
            DATA_STORE[keygroup] = {}
            VERSION_COUNTER[keygroup] = {}
            
        DATA_STORE[keygroup][item_id] = data
        
       
        current_version = VERSION_COUNTER[keygroup].get(item_id, 0) + 1
        VERSION_COUNTER[keygroup][item_id] = current_version
        
        item_version_map = {item_id: current_version}

        logging.info(f"MockFReD: Updated/Created item. Keygroup='{keygroup}', id='{item_id}', new_version={current_version}")
        return client_pb2.UpdateResponse(version=client_pb2.Version(version=item_version_map))

   
    def CreateKeygroup(self, request, context):
        logging.info(f"MockFReD: Received CreateKeygroup request for keygroup='{request.keygroup}'")
        if request.keygroup not in DATA_STORE:
            DATA_STORE[request.keygroup] = {}
            VERSION_COUNTER[request.keygroup] = {}
            logging.info(f"MockFReD: Keygroup '{request.keygroup}' created.")
        else:
            logging.info(f"MockFReD: Keygroup '{request.keygroup}' already exists.")
        return client_pb2.Empty()

    def DeleteKeygroup(self, request, context):
        logging.info(f"MockFReD: DeleteKeygroup request for keygroup='{request.keygroup}' - NOT IMPLEMENTED in mock")
        context.set_code(grpc.StatusCode.UNIMPLEMENTED)
        context.set_details('Method not implemented!')
        return client_pb2.Empty()

    def Scan(self, request, context):
        logging.info(f"MockFReD: Scan request - NOT IMPLEMENTED in mock")
        context.set_code(grpc.StatusCode.UNIMPLEMENTED)
        context.set_details('Method not implemented!')
        return client_pb2.ScanResponse()

    def Keys(self, request, context):
        logging.info(f"MockFReD: Keys request for keygroup='{request.keygroup}'")
        keygroup = request.keygroup
        keys_in_group = []
        if keygroup in DATA_STORE:
            for item_id in DATA_STORE[keygroup]:
                item_version_map = {}
                if item_id in VERSION_COUNTER.get(keygroup, {}):
                    item_version_map[item_id] = VERSION_COUNTER[keygroup][item_id]
                keys_in_group.append(client_pb2.Key(id=item_id, version=client_pb2.Version(version=item_version_map)))
        logging.info(f"MockFReD: Returning {len(keys_in_group)} keys for keygroup '{keygroup}'")
        return client_pb2.KeysResponse(keys=keys_in_group)
        
    def Delete(self, request, context):
        logging.info(f"MockFReD: Delete request for keygroup='{request.keygroup}', id='{request.id}'")
        keygroup = request.keygroup
        item_id = request.id
        deleted = False
        if keygroup in DATA_STORE and item_id in DATA_STORE[keygroup]:
            del DATA_STORE[keygroup][item_id]
            if item_id in VERSION_COUNTER.get(keygroup, {}):
                del VERSION_COUNTER[keygroup][item_id]
            deleted = True
            logging.info(f"MockFReD: Deleted item with id='{item_id}' from keygroup='{keygroup}'")
        else:
            logging.warning(f"MockFReD: Item not found for deletion: keygroup='{keygroup}', id='{item_id}'")

        
        return client_pb2.DeleteResponse(version=client_pb2.Version())


   
    def AddReplica(self, request, context):
        logging.info(f"MockFReD: AddReplica for kg='{request.keygroup}', node='{request.nodeId}' - Mocked, no action.")
        return client_pb2.Empty()

    def RemoveReplica(self, request, context):
        logging.info(f"MockFReD: RemoveReplica for kg='{request.keygroup}', node='{request.nodeId}' - Mocked, no action.")
        return client_pb2.Empty()
        
   
    def GetKeygroupInfo(self, request, context):
        context.set_code(grpc.StatusCode.UNIMPLEMENTED)
        context.set_details('Method not implemented in mock!')
        raise NotImplementedError('Method not implemented in mock!')

    def Append(self, request, context):
        context.set_code(grpc.StatusCode.UNIMPLEMENTED)
        context.set_details('Method not implemented in mock!')
        raise NotImplementedError('Method not implemented in mock!')

    def GetReplica(self, request, context):
        context.set_code(grpc.StatusCode.UNIMPLEMENTED)
        context.set_details('Method not implemented in mock!')
        raise NotImplementedError('Method not implemented in mock!')

    def GetAllReplica(self, request, context):
        context.set_code(grpc.StatusCode.UNIMPLEMENTED)
        context.set_details('Method not implemented in mock!')
        raise NotImplementedError('Method not implemented in mock!')

    def GetKeygroupTriggers(self, request, context):
        context.set_code(grpc.StatusCode.UNIMPLEMENTED)
        context.set_details('Method not implemented in mock!')
        raise NotImplementedError('Method not implemented in mock!')

    def AddTrigger(self, request, context):
        context.set_code(grpc.StatusCode.UNIMPLEMENTED)
        context.set_details('Method not implemented in mock!')
        raise NotImplementedError('Method not implemented in mock!')

    def RemoveTrigger(self, request, context):
        context.set_code(grpc.StatusCode.UNIMPLEMENTED)
        context.set_details('Method not implemented in mock!')
        raise NotImplementedError('Method not implemented in mock!')

    def AddUser(self, request, context):
        context.set_code(grpc.StatusCode.UNIMPLEMENTED)
        context.set_details('Method not implemented in mock!')
        raise NotImplementedError('Method not implemented in mock!')

    def RemoveUser(self, request, context):
        context.set_code(grpc.StatusCode.UNIMPLEMENTED)
        context.set_details('Method not implemented in mock!')
        raise NotImplementedError('Method not implemented in mock!')


def serve():
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
    client_pb2_grpc.add_ClientServicer_to_server(MockFredServicer(), server)
    server.add_insecure_port('[::]:9001') 
    logging.info("MockFReD gRPC server started on port 9001")
    server.start()
    try:
        while True:
            time.sleep(_ONE_DAY_IN_SECONDS)
    except KeyboardInterrupt:
        logging.info("MockFReD gRPC server stopping")
        server.stop(0)

if __name__ == '__main__':
    serve()