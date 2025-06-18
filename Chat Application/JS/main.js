const socket = io('http://localhost:8000');
const form = document.getElementById('send-container');
const messageInput = document.getElementById('messageInp');
const messageContainer = document.querySelector(".container");

var audio=new Audio('Message-notification.mp3');

const append = (message, position) => {
    const messageElement = document.createElement('div'); 
    messageElement.innerText = message;
    messageElement.classList.add('message', position); 
    messageContainer.append(messageElement);
    
    if(position=='left'){
        audio.play();
    }  
};

form.addEventListener('submit',(e)=>{
    e.preventDefault();
    const message = messageInput.value; 
    append(`You : ${message}`,'right');
    socket.emit('send', message);
    messageInput.value = '';
});

// Prompt for username and emit to server
const Username = prompt("Enter Your Name to Join");
socket.emit('new-user-joined', Username);

// Listen for new user joining
socket.on('user-joined', name => {
    append(`${name} joined the chat`, 'right');
});

socket.on('receive', data => {
    append(`${data.Username}: ${data.message}`, 'left');
});

socket.on('left', name => {
    append(`${name} left the chat`, 'left');
});



