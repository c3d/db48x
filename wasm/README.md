# DB48x WebAssembly App

A WebAssembly port of the DB48x calculator.
## Running

In the parent directory execute:
```
make wasm
```

In the current directory execute:
```shell
npm install
npm run dev
```

Open the url [http://localhost:5173](http://localhost:5173) with a web browser.

## Modify keyboard skin or key placement

If there is the need to update the keyboard skin at some point, these are the instructions about how to do it:

1. Open the file `keyboard-inkscape.svg` with [Inkscape](https://inkscape.org/).
1. Move the rectangle objects to the proper place or create new ones if needed.
1. For each rectangle object, right click and select `Object Properties...`.
1. In the dialogue, set the `ID` property to any code from the variable `KB_CMD` (see this variable in `index.html` file).
1. Click `interactivity` tab and be sure to set `onmousedown` and `onmouseup` to the values `keyboard.onMouseDownHandler(this.id);` and `keyboard.onMouseUpHandler(this.id);` respectively.
1. Click `File->Export`, set `Page` and export it as Plain SVG(*.svg). Note: be sure to remove the keyboard image from the document so the document only contains the rectangles.
1. Save the file to the `public/` directory.
1. Manually copy the svg element in that file to the proper place in `index.html` file.

