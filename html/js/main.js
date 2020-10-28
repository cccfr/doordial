// var base_url = 'http://localhost';
var base_url = 'https://sesam.cccfr:8443'; // baseurl

var domReady = (callback) => {
    if (document.readyState != "loading") callback();
    else document.addEventListener("DOMContentLoaded", callback);
}

domReady(() => {
    document.addEventListener('click',function onclicked(e){
        documentClicked(e);
    });
    main();
});

function main() {
    getDataFromUrl(base_url + '/api/cards');
    getDataFromUrl(base_url + '/api/unknowns');
}

function documentClicked(e) {
    // console.log(e);

    if (e.target.classList.contains('rfid-choose-field')) {
        e.preventDefault();
        document.getElementById('rfid_input').value=e.target.dataset.rfid;
    }

    if (e.target.classList.contains('del-user-btn')) {
        e.preventDefault();
        console.log(e);
        if (window.confirm(`DELETE?\r\n\RFID: ${e.target.dataset.rfid} \r\nUser: ${e.target.dataset.user}\r\n`)) {
            sendDataToUrl(base_url + '/api/delcard', {Rfid: e.target.dataset.rfid, User: e.target.dataset.user, Pin: '0000'});
            console.log('deleted');
        }
    }

    if (e.target.classList.contains('reset-pin-btn')) {
        e.preventDefault();
        // console.log(e);
        if (window.confirm(`REST PIN?\r\n\RFID: ${e.target.dataset.rfid} \r\nUser: ${e.target.dataset.user}\r\n`)) {
            sendDataToUrl(base_url + '/api/newcard?id=' + e.target.dataset.rfid, {Rfid: e.target.dataset.rfid, User: e.target.dataset.user, Pin: '0000'});
        }
    }

    if (e.target.classList.contains('form-btn-add')) {
        e.preventDefault();
        if ( e.target.form.User.value == "" || e.target.form.Rfid.value == "" ) {
            window.alert('rfid AND nick needed!');
            return;
        }
        sendDataToUrl(base_url + '/api/newcard', {Rfid: e.target.form.Rfid.value, User: e.target.form.User.value, Pin: '0000'});
        console.log('new user added');
    }
}

function addUserList(userlist) {
    let cont = '';
    userlist = localeSort(userlist, j => j.User)
    Object.entries(userlist).forEach(([key, value]) => {
        if (value.Rfid == '' || value.User == '') { return; }
        cont += `<li data-rfid="${value.Rfid}">${value.User} ${value.Rfid}
            <button class="reset-pin-btn" data-rfid="${value.Rfid}" data-user="${value.User}">reset pin</button>
            <button class="del-user-btn" data-rfid="${value.Rfid}" data-user="${value.User}">X</button>
        </li>`;
    })
    if (cont == '') { cont = '<li>no users found</li>'};
    document.getElementById('user_list').innerHTML = cont;
}

function addUnknownsList(userlist) {
    let cont = '';
    Object.entries(userlist).forEach(([key, value]) => {
        if (value == '') { return; }
        cont += `<li>
            <button data-rfid="${value}" class="rfid-choose-field">${value}</button>
        </li>`;
    })
    if (cont == '') { cont = '<li>no users found</li>'};
    document.getElementById('rfid_last_unknown').innerHTML = cont;
}

function dataArrived(dataAr, source = '') {
    if(/.\/api\/cards$/.test(source)) {
        addUserList(dataAr);
        return;
    }

    if(/.\/api\/unknowns$/.test(source)) {
        addUnknownsList(dataAr);
        return;
    }

    console.log('not found: ' + source);
}

function sendingAnswer(data, source = '') {
    // console.log(data);
    // console.log(source);

    if (data.status == 200) {

        if(/.\/api\/newcard$/.test(source)) {
            document.getElementById('rfid_input').value = '';
            document.getElementById('user_input').value = '';
            document.getElementById('rfid_input').focus();
            getDataFromUrl(base_url + '/api/cards');
            getDataFromUrl(base_url + '/api/unknowns');
            return;
        }

        let found = /\/api\/newcard\?id=(.+)/g.exec(source);
        if(found != null && typeof found[1] !== 'undefined') {
            document.querySelectorAll(`.reset-pin-btn[data-rfid="${found[1]}"]`).forEach(function(target){
                target.innerHTML = 'Pin reseted';
            });
            return;
        }

        if(/.\/api\/delcard$/.test(source)) {
            getDataFromUrl(base_url + '/api/cards');
            getDataFromUrl(base_url + '/api/unknowns');
            return;
        }
    }
}

function getDataFromUrl(url) {
    fetch(url, {headers: {"Content-type": "application/json; charset=UTF-8"}}).then(response => {
    if (response.ok) {
        return Promise.resolve(response, url);
    }
    else {
        return Promise.reject(new Error('Failed to load')); 
    }
    })
    .then(response => response.json(), url) // parse response as JSON
    .then(data => {
        dataArrived(data, url);
        // console.log(url);
    })
    .catch(function(error) {
        alert('Sorry, das ging schief, es konnten keine oder nur defekte Daten empfangen werden.');
        console.log(`Error: ${error.message}`);
    });
}

function sendDataToUrl(url, dataAr) {
    fetch(url, {
        method: "POST",
        body: JSON.stringify(dataAr),
        headers: {"Content-type": "application/json; charset=UTF-8"}
    }).then(response => {
    if (response.ok) {
        return Promise.resolve(response, url);
    }
    else {
        return Promise.reject(new Error('Failed to load')); 
    }
    })
    .then(response => {sendingAnswer(response, url)})
    .catch(function(error) {
        alert('Sorry, das ging schief, es konnten keine oder nur defekte Daten empfangen werden.');
        console.log(`Error: ${error.message}`);
    });
}


function localeSort(items, it) {
    return items.slice().sort((a, b) => {
        const valueA = String(it(a))
        const valueB = String(it(b))
        return valueA.localeCompare(valueB, undefined, {
            ignorePunctuation: true,
            numeric: true,
            sensitivity: 'base',
        })
    })
}
