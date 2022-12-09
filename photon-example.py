import photon
import pixpy as pix

screen = pix.open_display(width=640, height=480)

pos = pix.Vec2(0, screen.size.y/2)

APP_ID = "5569cb7b-b875-41ab-9155-a44de7c4c227"
client = photon.create_client(APP_ID)
client.connect()

while pix.run_loop():
    match client.update():
        case photon.event.Connected(no):
            print("CONNECTED")
            client.join_room('game')
        case photon.event.PlayerJoined(no, player):
            print(f"Player {player.id} joined")
        case photon.event.Data(payload):
            print(f"Got {payload}")

    if pix.was_pressed('a'):
        client.send_event(5, 'data', reliable=True)
    screen.clear()
    screen.circle(center=pos, radius=pos.x/4)
    pos += (1, 0)
    screen.swap()
