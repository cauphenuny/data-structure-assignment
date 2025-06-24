== GUI

#figure(image("assets/knights-gui.png"))

使用第三方库 `SFML`

#figure(image("assets/deepwiki.png", width: 70%))

#pagebreak()

一个 bug

```cpp
ChessboardRenderer::ChessboardRenderer(sf::Font& font)
    : font(font),
      running(false),
      ...
      currentStep(0),
      animProgress(0.0f),
      knight(knightTexture), // knightTexture not constructed!
      lastRenderedStep(-1),
      lastRenderedSolution(-1) {
```