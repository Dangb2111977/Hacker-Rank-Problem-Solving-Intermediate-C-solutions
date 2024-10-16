const express = require('express');
const http = require('http');
const socketIo = require('socket.io');
const cors = require('cors'); // Importing CORS if needed

const app = express();
const server = http.createServer(app);
const io = socketIo(server);

// Middleware
app.use(cors()); // Enable CORS if necessary
app.use(express.json());
app.use(express.static('public')); // Serve static files from the 'public' directory

// Route for the homepage
app.get('/', (req, res) => {
    res.sendFile(__dirname + '/public/index.html'); // Return the index.html file
});

// Route to store solutions
app.post('/api/solutions', (req, res) => {
    try {
        const { user, problem, code, thoughtProcess } = req.body;

        // Check required fields
        if (!user || !problem || !code) {
            return res.status(400).json({ error: 'Username, problem, and code are required.' });
        }

        // Log the received solution
        console.log('Received data:', { user, problem, code, thoughtProcess });

        // Respond to client
        res.status(201).json({ message: 'Solution saved successfully!', solution: req.body });
    } catch (error) {
        console.error('Error saving solution:', error);
        res.status(500).json({ error: 'Internal Server Error' });
    }
});

// Socket.io connection
io.on('connection', (socket) => {
    console.log('A user connected');

    // Handle chat messages
    socket.on('chat message', (msg) => {
        console.log('Chat message received:', msg);
        io.emit('chat message', msg); // Broadcast the message to all connected clients
    });

    socket.on('disconnect', () => {
        console.log('User disconnected');
    });
});

// Start the server
server.listen(8080, () => {
    console.log('Server listening on port 8080');
});
