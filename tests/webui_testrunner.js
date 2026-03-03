const { Window } = require("happy-dom");

// Initialize the Window and Document
const window = new Window({
  url: 'http://localhost',
  settings: {
    disableJavaScriptFileLoading: false,
    disableJavaScriptEvaluation: false,
    disableCSSFileLoading: false
  }
});

const document = window.document;
document.write('<!DOCTYPE html><html><body></body></html>');

// Polyfill globals for Embind / Emscripten
global.window = window;
global.document = document;
global.Node = window.Node;
global.HTMLElement = window.HTMLElement;
global.navigator = window.navigator;

// Happy DOM requires manual linking of some globals that Emscripten expects
global.requestAnimationFrame = window.requestAnimationFrame.bind(window);
global.cancelAnimationFrame = window.cancelAnimationFrame.bind(window);

// Mock bounding box since Happy DOM doesn't have a layout engine and will not compute the values like the browser does.
global.HTMLElement.prototype.getBoundingClientRect = function() {
  const lines = 1 + Array.from(this.textContent.matchAll(/\s./sg)).length;
  const height = lines * 50;
  return {
    x: 0,
    y: 0,
    width: 100, // Mocked value
    height: height, // Mocked value
    top: 50,
    right: 0,
    bottom: 0,
    left: 100
  };
};

// Load and run emscripten generated test files
const Module = require("./build/web/WebUi_tests.js");

Module.onRuntimeInitialized = () => {
  console.log("Running WebUI tests with Happy DOM...");
};