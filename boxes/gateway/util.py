import click
import serial

@click.command()
@click.option("--port", type=click.Path())
def read_cards(port):
    with serial.Serial(port, 115200, timeout=30) as ser:
        try:
            ser.write("q\nactivate\n".encode("utf-8"))
            ser.readline()
            ser.read_all()
            while True:
                ser.read_all()
                ser.write("read_card\n".encode("utf-8"))
                while True:
                    line = ser.readline().decode("utf-8").strip()
                    if line == "OK":
                        break
                    print(line)
                print("")
        finally:
            ser.write("q\ndeactivate\n".encode("utf-8"))

@click.command()
@click.option("--port", type=click.Path())
def clear_cards(port):
    with serial.Serial(port, 115200, timeout=30) as ser:
        try:
            ser.write("q\nactivate\n".encode("utf-8"))
            ser.readline()
            ser.read_all()
            while True:
                ser.read_all()
                ser.write("clear_card\n".encode("utf-8"))
                while True:
                    line = ser.readline().decode("utf-8").strip()
                    if line == "OK":
                        break
                    print(line)
                print("")
        finally:
            ser.write("q\ndeactivate\n".encode("utf-8"))

@click.command()
@click.option("--port", type=click.Path())
@click.option("--team", type=int, help="Team number")
@click.option("--start-idx", type=int, default=0, help="Start index for cards")
def write_cards(port, team, start_idx):
    with serial.Serial(port, 115200, timeout=30) as ser:
        try:
            ser.write("activate\n".encode("utf-8"))
            ser.readline()
            while True:
                ser.read_all()
                print(f"Writing card for team {team}; SEQ {start_idx}")
                ser.write(f"write_card:{team}:{start_idx}\n".encode("utf-8"))
                while True:
                    line = ser.readline().decode("utf-8").strip()
                    if line == "OK":
                        break
                    print(line)
                print("Card written successfully")
                start_idx += 1
        finally:
            ser.write("q\ndeactivate\n".encode("utf-8"))

@click.group()
def cli():
    pass

cli.add_command(read_cards)
cli.add_command(write_cards)
cli.add_command(clear_cards)

if __name__ == "__main__":
    cli()
