const express = require('express');
const fs = require('fs');
const path = require('path');
const { v4: uuidv4 } = require('uuid');

const app = express();
const PORT = 3000;

app.use(express.json());
app.use(express.static('public'));

const TASK_FILE = 'tasks.json';

function readTasks() {
  if (!fs.existsSync(TASK_FILE)) return [];
  return JSON.parse(fs.readFileSync(TASK_FILE));
}

function writeTasks(tasks) {
  fs.writeFileSync(TASK_FILE, JSON.stringify(tasks, null, 2));
}

app.get('/tasks', (req, res) => {
  res.json(readTasks());
});

app.post('/tasks', (req, res) => {
  const tasks = readTasks();
  const newTask = { id: uuidv4(), text: req.body.text, done: false };
  tasks.push(newTask);
  writeTasks(tasks);
  res.status(201).json(newTask);
});

app.delete('/tasks/:id', (req, res) => {
  let tasks = readTasks();
  tasks = tasks.filter(task => task.id !== req.params.id);
  writeTasks(tasks);
  res.status(204).end();
});

app.put('/tasks/:id', (req, res) => {
  const tasks = readTasks();
  const task = tasks.find(t => t.id === req.params.id);
  if (task) task.done = !task.done;
  writeTasks(tasks);
  res.json(task);
});

app.listen(PORT, () => console.log(`🟢 Server running at http://localhost:${PORT}`));
