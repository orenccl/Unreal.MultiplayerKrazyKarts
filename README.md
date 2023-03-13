# Unreal.MultiplayerKrazyKarts
Familiarize with the workflow of the Unreal Engine Multiplayer by creating a simple Karts game.

* Basic Car Physics(Air Resistance、Rolling Resistance、Steering And Turning Circles)
* Server Functions(RPC) & Cheat Protection
* Network roles(Authority、AutonomousProxy、SimulatedProxy)
* Deal with package lag

Under normal conditions, the client and server synchronize properly.
![image](https://github.com/orenccl/Unreal.MultiplayerKrazyKarts/blob/main/Preview/NormalCondition.gif)

When the client experiences network lag (i.e. can only send one package per second), the client will move ahead of the server and mark the move as an unacknowledged move. Once the client receives a package from the server, it will synchronize to the correct position and replay the unacknowledged move, continuing to move ahead of the server.
![image](https://github.com/orenccl/Unreal.MultiplayerKrazyKarts/blob/main/Preview/ClientLag.gif)

When the server experiences network lag (i.e. can only send one package per second), the client will immediately update its simulated proxy position upon receiving the package, and simulate moves from the last position.
![image](https://github.com/orenccl/Unreal.MultiplayerKrazyKarts/blob/main/Preview/ServerNetLag.gif)
