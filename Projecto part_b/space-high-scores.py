import zmq
from score_pb2 import ScoreUpdate  # Generated from score.proto

# Initialize ZeroMQ context and socket
context = zmq.Context()
socket = context.socket(zmq.SUB)
socket.bind("tcp://*:5555")
socket.setsockopt_string(zmq.SUBSCRIBE, "")  # Subscribe to all messages

print("Server is running and waiting for scores...")

while True:
    # Receive the serialized message
    message = socket.recv()

    # Deserialize using Protocol Buffers
    score_update = ScoreUpdate()
    score_update.ParseFromString(message)
    players="ABCDEFGH"
    # Print the received arrays
    print(f"Astronaut Scores: {score_update.astronaut_scores}")
    print(f"Space Mission Scores: {score_update.space_mission_scores}")
    print("%%%%%%%%%%%%%%%%%%% HIGH SCORES%%%%%%%%%%%%%%%%%%% ")
    cnt=0
    for x in score_update.astronaut_scores:
        if x!=0:
            print("PLayer ", players[cnt]," with score",score_update.space_mission_scores[cnt],"\n")
        cnt = cnt + 1

