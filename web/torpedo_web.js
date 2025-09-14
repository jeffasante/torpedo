let torpedoModule = null;
let lastEncodedUrl = null;
let lastPassword = null;

// Load WASM module
TorpedoModule().then(module => {
    torpedoModule = module;
    console.log('Torpedo WASM loaded');
}).catch(err => {
    console.error('Failed to load WASM:', err);
});

function loadImageToCanvas(file) {
    return new Promise((resolve, reject) => {
        const img = new Image();
        img.onload = () => {
            const canvas = document.createElement('canvas');
            const ctx = canvas.getContext('2d');
            
            canvas.width = img.width;
            canvas.height = img.height;
            ctx.drawImage(img, 0, 0);
            
            const imageData = ctx.getImageData(0, 0, img.width, img.height);
            resolve({
                width: img.width,
                height: img.height,
                data: imageData.data
            });
        };
        img.onerror = reject;
        img.src = URL.createObjectURL(file);
    });
}

function canvasToBlob(width, height, rgbaData, format = 'image/png') {
    const canvas = document.createElement('canvas');
    const ctx = canvas.getContext('2d');
    
    canvas.width = width;
    canvas.height = height;
    
    const imageData = ctx.createImageData(width, height);
    imageData.data.set(rgbaData);
    ctx.putImageData(imageData, 0, 0);
    
    return new Promise(resolve => {
        canvas.toBlob(resolve, format, 0.95);
    });
}

async function encodeMessage() {
    if (!torpedoModule) {
        document.getElementById('encodeResult').innerHTML = '<p class="error">WASM module not loaded yet.</p>';
        return;
    }
    
    const imageFile = document.getElementById('imageInput').files[0];
    const message = document.getElementById('messageInput').value;
    const password = document.getElementById('passwordInput').value || null;
    
    if (!imageFile || !message) {
        document.getElementById('encodeResult').innerHTML = '<p class="error">Please select an image and enter a message.</p>';
        return;
    }
    
    try {
        const { width, height, data } = await loadImageToCanvas(imageFile);
        
        const dataPtr = torpedoModule._malloc(data.length);
        const outputPtr = torpedoModule._malloc(data.length);
        const messagePtr = torpedoModule._malloc(message.length + 1);
        const passwordPtr = password ? torpedoModule._malloc(password.length + 1) : 0;
        
        new Uint8Array(torpedoModule.HEAPU8.buffer, dataPtr, data.length).set(data);
        torpedoModule.stringToUTF8(message, messagePtr, message.length + 1);
        if (password) torpedoModule.stringToUTF8(password, passwordPtr, password.length + 1);
        
        const result = torpedoModule._torpedo_encode_js(dataPtr, width, height, messagePtr, passwordPtr, outputPtr);
        
        if (result === 0) {
            const encodedData = new Uint8Array(data.length);
            encodedData.set(new Uint8Array(torpedoModule.HEAPU8.buffer, outputPtr, data.length));
            const blob = await canvasToBlob(width, height, encodedData, 'image/png');
            const url = URL.createObjectURL(blob);
            
            lastEncodedUrl = url;
            lastPassword = password;
            
            document.getElementById('encodeResult').innerHTML = 
                `<p class="success">Message encoded successfully!</p>
                 <a href="${url}" download="stego.png">Download Stego Image</a>
                 <button onclick="quickDecode()">Quick Decode</button>`;
        } else {
            document.getElementById('encodeResult').innerHTML = '<p class="error">Encoding failed.</p>';
        }
        
        torpedoModule._free(dataPtr);
        torpedoModule._free(outputPtr);
        torpedoModule._free(messagePtr);
        if (passwordPtr) torpedoModule._free(passwordPtr);
        
    } catch (error) {
        document.getElementById('encodeResult').innerHTML = '<p class="error">Error: ' + error.message + '</p>';
    }
}

async function decodeMessage() {
    if (!torpedoModule) {
        document.getElementById('decodeResult').innerHTML = '<p class="error">WASM module not loaded yet.</p>';
        return;
    }
    
    const imageFile = document.getElementById('stegoInput').files[0];
    const password = document.getElementById('decodePasswordInput').value || null;
    
    if (!imageFile) {
        document.getElementById('decodeResult').innerHTML = '<p class="error">Please select a stego image.</p>';
        return;
    }
    
    try {
        const { width, height, data } = await loadImageToCanvas(imageFile);
        
        const dataPtr = torpedoModule._malloc(data.length);
        const messagePtr = torpedoModule._malloc(1000);
        const passwordPtr = password ? torpedoModule._malloc(password.length + 1) : 0;
        
        new Uint8Array(torpedoModule.HEAPU8.buffer, dataPtr, data.length).set(data);
        if (password) torpedoModule.stringToUTF8(password, passwordPtr, password.length + 1);
        
        const result = torpedoModule._torpedo_decode_js(dataPtr, width, height, passwordPtr, messagePtr, 1000);
        
        if (result === 0) {
            const decodedMessage = torpedoModule.UTF8ToString(messagePtr);
            document.getElementById('decodeResult').innerHTML = 
                `<p class="success">Decoded message:</p><pre>${decodedMessage}</pre>`;
        } else {
            document.getElementById('decodeResult').innerHTML = '<p class="error">Decoding failed or no message found.</p>';
        }
        
        torpedoModule._free(dataPtr);
        torpedoModule._free(messagePtr);
        if (passwordPtr) torpedoModule._free(passwordPtr);
        
    } catch (error) {
        document.getElementById('decodeResult').innerHTML = '<p class="error">Error: ' + error.message + '</p>';
    }
}

async function quickDecode() {
    if (!torpedoModule || !lastEncodedUrl) {
        document.getElementById('decodeResult').innerHTML = '<p class="error">No encoded image available.</p>';
        return;
    }
    
    try {
        const response = await fetch(lastEncodedUrl);
        const blob = await response.blob();
        const file = new File([blob], 'stego.png', { type: 'image/png' });
        
        const { width, height, data } = await loadImageToCanvas(file);
        
        const dataPtr = torpedoModule._malloc(data.length);
        const messagePtr = torpedoModule._malloc(1000);
        const passwordPtr = lastPassword ? torpedoModule._malloc(lastPassword.length + 1) : 0;
        
        const dataView = new Uint8Array(torpedoModule.HEAPU8.buffer, dataPtr, data.length);
        dataView.set(data);
        if (lastPassword) torpedoModule.stringToUTF8(lastPassword, passwordPtr, lastPassword.length + 1);
        
        const result = torpedoModule._torpedo_decode_js(dataPtr, width, height, passwordPtr, messagePtr, 1000);
        
        if (result === 0) {
            const decodedMessage = torpedoModule.UTF8ToString(messagePtr);
            document.getElementById('decodeResult').innerHTML = 
                `<p class="success">Quick decoded:</p><pre>${decodedMessage}</pre>`;
        } else {
            document.getElementById('decodeResult').innerHTML = '<p class="error">Quick decode failed.</p>';
        }
        
        torpedoModule._free(dataPtr);
        torpedoModule._free(messagePtr);
        if (passwordPtr) torpedoModule._free(passwordPtr);
        
    } catch (error) {
        document.getElementById('decodeResult').innerHTML = '<p class="error">Quick decode error: ' + error.message + '</p>';
    }
}