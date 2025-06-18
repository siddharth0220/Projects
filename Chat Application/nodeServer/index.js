//Node server which will be responsible for handling Socket.io connections.
const io = require('socket.io')(8000, {
    cors: {
        origin: "http://127.0.0.1:3000",
        methods: ["GET", "POST"]
    }
});

// const io= require('socket.io')(8000)
//Server which will be responsible for handling HTTP requests with port number 8000.
const users={};

io.on('connection',socket=>{
    socket.on('new-user-joined',Username=>{
        users[socket.id]=Username;
socket.broadcast.emit('user-joined',Username);

    });
    socket.on('send',message=>{
        socket.broadcast.emit('receive',{message: message,Username: users[socket.id]})
    });
    socket.on('disconnect',message=>{
        socket.broadcast.emit('left',users[socket.id]);
        delete users[socket.id];
    });
});

