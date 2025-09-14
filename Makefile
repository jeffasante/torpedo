CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2
SRCDIR = src
SOURCES = $(SRCDIR)/torpedo.c $(SRCDIR)/image.c $(SRCDIR)/lsb.c $(SRCDIR)/crypto.c $(SRCDIR)/main.c
TARGET = torpedo

.PHONY: all clean wasm web test

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(TARGET) web/torpedo.js web/torpedo.wasm

wasm:
	emcc -O2 -s WASM=1 \
		-s EXPORTED_FUNCTIONS='["_torpedo_encode_js", "_torpedo_decode_js", "_torpedo_get_capacity_js", "_malloc", "_free"]' \
		-s EXPORTED_RUNTIME_METHODS='["ccall", "cwrap", "getValue", "setValue"]' \
		-s ALLOW_MEMORY_GROWTH=1 \
		-s MODULARIZE=1 \
		-s EXPORT_NAME='TorpedoModule' \
		$(SRCDIR)/torpedo.c $(SRCDIR)/image.c $(SRCDIR)/lsb.c $(SRCDIR)/crypto.c $(SRCDIR)/canvas_image.c $(SRCDIR)/wasm_wrapper.c \
		-o web/torpedo.js

web: wasm
	@echo "WebAssembly build complete. Open web/index.html in browser."

demo:
	@echo "Opening demo page (no WASM required)..."
	@open web/demo.html || xdg-open web/demo.html || echo "Open web/demo.html in your browser"

test: $(TARGET)
	@echo "Running basic tests..."
	@echo "Test message" > test_data.txt
	./$(TARGET) encode -i tests/test_images/test.bmp -o test_output.bmp -d test_data.txt
	./$(TARGET) decode -i test_output.bmp -o decoded.txt
	@if cmp -s test_data.txt decoded.txt; then \
		echo "✓ Basic encode/decode test passed"; \
	else \
		echo "✗ Basic encode/decode test failed"; \
	fi
	@rm -f test_data.txt test_output.bmp decoded.txt