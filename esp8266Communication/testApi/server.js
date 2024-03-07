const express = require('express');
const app = express();

const path = require('path');


const bodyParser = require('body-parser');

app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: true }));


const devices = [
    {
        id: 0,
        isOpen: true,
        codeFromUser: 0
    },
    {
        id: 1,
        isOpen: true,
        codeFromUser: 0
    },
];


app.get('/devices', (req, res) =>
    res.json(devices));

app.get('/devices/:id', (req, res) => {
    const { id } = req.params;
    const device = devices.find(element => element.id == id);
    res.json(device);
});



app.post("/lock", (req, res) => {
    const { id, codeFromUser } = req.body;
    const device = devices.find(element => element.id == parseInt(id));

    console.log("Lock device id: ", device.id);

    if (device.codeFromUser != 0) {
        return res.sendStatus(401);
    }

    device.isOpen = false;
    device.codeFromUser = codeFromUser;

    res.sendStatus(200);
})

app.get('/', function (req, res) {
    res.sendFile(path.join(__dirname, '/index.html'));
});




app.post("/unlock", (req, res) => {
    const { id, codeFromUser } = req.body;
    const device = devices.find(element => element.id == id);

    console.log("Unlock device id: ", device.id);

    if (device.codeFromUser != codeFromUser) {
        return res.sendStatus(401)
    }

    device.isOpen = true;
    device.codeFromUser = 0;
    res.sendStatus(200);

})



const server = app.listen(3001, () => {
    console.log('listening on port %s...', server.address().port);
});

