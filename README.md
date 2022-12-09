## PIX (pixpy)

A graphics library with a python interface.
Designed for learning and 2D game development.

* Uses OpenGL/GLES2 to make it fast and portable
* Efficient Console/TileSet rendering for tile or text based games
* Composable Images using only GL textures

### Install

```sh
pip install pixpy
```

For Linux, we need to build from source so dependencies must be installed first;

```sh
sudo apt install libxinerama-dev libxi-dev libxrandr-dev libxcursor-dev
```

### The Basics

The following is a full program that opens a window and draws a circle;

```python
import pixpy as pix
screen = pix.open_display(size=(1280,720))
screen.circle(center=(640,360), radius=300)
```

**NOTE:** This simple example works because pix is smart enough to "swap" the screen to automatically display what you have drawn, and then leave the window open and wait for the user to close the window, before the script ends.

Normally you create your own main loop and do this yourself;

```python
import pixpy as pix

screen = pix.open_display(width=1280, height=720)

x = 0
while pix.run_loop():
    screen.clear()
    screen.circle(center=(x,360), radius=x/4)
    x += 1
    screen.swap()
```

To read the keyboard and/or mouse, you can use _is_pressed()_ or _was_pressed()_

```python
import pixpy as pix

screen = pix.open_display(width=640, height=480)

background = pix.load_png("data/background.png")
sprite = pix.load_png("data/ufo.png")

pos = pix.Float2(screen.size.x/2, screen.size.y - 50)

while pix.run_loop():
    screen.draw(image=background, size=screen.size)
    if pix.is_pressed(pix.key.RIGHT):
        pos += (2,0)
    elif pix.is_pressed(pix.key.LEFT):
        pos -= (2,0)
    screen.draw(image=sprite, center=pos)
    screen.swap()
```

For more advanced needs you use events

```python
import pixpy as pix

screen = pix.open_display(width=1280, height=720)
canvas = pix.Image(size=screen.size)

while pix.run_loop():
    for e in pix.all_events():
        if type(e) == pix.event.Click:
            canvas.context.filled_circle(center=e.pos, radius=15)
    screen.draw(image=canvas)
    screen.swap()
```

### The Console

A major part of pix is the _Console_

In its simplest form, it can be used for text output

The console needs to be drawn to be visible, just like everything else.

```python
import pixpy as pix

screen = pix.open_display(width=1280, height=720)
con = pix.Console(cols=80, rows=50)
con.write('Hello\n')
con.render(screen)
```

`console.read_line()` can be used to read lines of text. The result will be posted
as a TextEvent.


```python
import pixpy as pix

screen = pix.open_display(width=1280, height=720)
con = pix.Console(cols=40, rows=25)
con.write('What is your name?\n')
con.read_line()
while pix.run_loop():
    match pix.get_event():
        case pix.event.Text(text):
            con.write("Hello " + text)
            con.read_line()

    con.render(screen)
    screen.swap()
```

### The Core Objects

* A _Float2_ is a 2D dimensional vector with an _x_ and _y_ field that are used
to represent 2D coordinates and vectors.

* An _Image_ is a reference to a rectangular array of pixels on the GPU, or in other words, a set of 4 UV coordinates (known to form a rectangle) and a texture reference.

* The _Screen_ represents the window or display.

* A _Context_ ties together rendering _state_ with a rendering _target_. The Screen, and all Images, can be treated as context and can be drawn to.
