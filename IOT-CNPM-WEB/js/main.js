var stateLamp = false;
var valueLamp = "OFF";
function WriteDataToFirebase(lamp){
    firebase.database().ref("EQUIPMENT LIVINGROOM").set({
        Light1:lamp
    });
}
var dbRefLamp = firebase.database().ref('EQUIPMENT LIVINGROOM').child('Light1');
dbRefLamp.on('value', snap =>{
    if(snap.val()=="ON"){
        stateLamp=true;
        valueLamp="ON";
    }else if(snap.val()=="OFF"){
        stateLamp=false;
        valueLamp="OFF";
    }
});
function ClickLamp(){
    stateLamp=!stateLamp;
    if(stateLamp){
        valueLamp="ON";
    }else{
        valueLamp="OFF";
    }
    WriteDataToFirebase(valueLamp);
}

const temp =  document.getElementById("tvTemp");
const getData = async() =>{
    let lastDate,lastTime
    await firebase.database().ref("LIVING ROOM")
    .orderByKey().limitToLast(1)
    .once("value", (snapshot) =>{
        snapshot.forEach(function(data) {
            lastDate = data.val();
            lastTime = Object.keys(lastDate).sort(()=>-1)[0]
        });
    });
    return lastDate[lastTime].TEMPERATURE;
}
setTimeout(async()=>{
    let data = await getData();
    temp.innerHTML = data
},1)

setInterval(async()=>{
        let data = await getData()||0;
        temp.innerHTML = data
},1000*60)

const humi = document.getElementById("tvHumi");
const getHumi = async() =>{
    let lastDate,lastTime
    await firebase.database().ref("LIVING ROOM")
    .orderByKey().limitToLast(1)
    .once("value", (snapshot) =>{
        snapshot.forEach(function(data1) {
            lastDate = data1.val();
            lastTime = Object.keys(lastDate).sort(()=>-1)[0]
        });
    });
    return lastDate[lastTime].HUMIDITY;
}
setTimeout(async()=>{
    let data1 = await getHumi();
    humi.innerHTML = data1
},1)

setInterval(async()=>{
        let data1 = await getHumi()||0;
        humi.innerHTML = data1
},1000*60)

const temp1 =  document.getElementById("tvTemp1");
const getTemp = async() =>{
    let lastDate,lastTime
    await firebase.database().ref("BED ROOM")
    .orderByKey().limitToLast(1)
    .once("value", (snapshot) =>{
        snapshot.forEach(function(data2) {
            lastDate = data2.val();
            lastTime = Object.keys(lastDate).sort(()=>-1)[0]
        });
    });
    return lastDate[lastTime].TEMPERATURE1;
}
setTimeout(async()=>{
    let data2 = await getTemp();
    temp1.innerHTML = data2
},1)

setInterval(async()=>{
        let data2 = await getTemp()||0;
        temp1.innerHTML = data2
},1000*60)

const gas =  document.getElementById("tvGas");
const getGas = async() =>{
    let lastDate,lastTime
    await firebase.database().ref("KITCHEN ROOM")
    .orderByKey().limitToLast(1)
    .once("value", (snapshot) =>{
        snapshot.forEach(function(data3) {
            lastDate = data3.val();
            lastTime = Object.keys(lastDate).sort(()=>-1)[0]
        });
    });
    return lastDate[lastTime].GAS;
}
setTimeout(async()=>{
    let data3 = await getGas();
    gas.innerHTML = data3
},1)

setInterval(async()=>{
        let data3 = await getGas()||0;
        gas.innerHTML = data3
},1000*60)

const humi1 =  document.getElementById("tvHumi1");
const getHumi1 = async() =>{
    let lastDate,lastTime
    await firebase.database().ref("KITCHEN ROOM")
    .orderByKey().limitToLast(1)
    .once("value", (snapshot) =>{
        snapshot.forEach(function(data4) {
            lastDate = data4.val();
            lastTime = Object.keys(lastDate).sort(()=>-1)[0]
        });
    });
    return lastDate[lastTime].HUMIDITY1;
}
setTimeout(async()=>{
    let data4 = await getHumi1();
    humi1.innerHTML = data4
},1)

setInterval(async()=>{
        let data4 = await getHumi1()||0;
        humi1.innerHTML = data4
},1000*60)

const temp2 =  document.getElementById("tvTemp2");
const getTemp2 = async() =>{
    let lastDate,lastTime
    await firebase.database().ref("KITCHEN ROOM")
    .orderByKey().limitToLast(1)
    .once("value", (snapshot) =>{
        snapshot.forEach(function(data5) {
            lastDate = data5.val();
            lastTime = Object.keys(lastDate).sort(()=>-1)[0]
        });
    });
    return lastDate[lastTime].TEMPERATURE2;
}
setTimeout(async()=>{
    let data5 = await getTemp2();
    temp2.innerHTML = data5
},1)

setInterval(async()=>{
        let data5 = await getTemp2()||0;
        temp2.innerHTML = data5
},1000*60)

const btnLogin= document.getElementById("btnLogin")

const loginForm=document.getElementById("loginForm");

const btnLogout=document.getElementById("btnLogout");
const homePage=document.getElementById("homePage");
const backDrop=document.getElementById("backDrop")

firebase.auth().onAuthStateChanged((user) => {
    if (user) {
      // User is signed in, see docs for a list of available properties
      // https://firebase.google.com/docs/reference/js/firebase.User
      var uid = user.uid;
      loginForm.style.display="none";
      homePage.style.display="block";
      backDrop.style.display="block"

    } else {
    loginForm.style.display="block";
    homePage.style.display="none";
    
    }
  });


btnLogin.addEventListener('click',function () {
    let username=document.getElementById("username").value;
    let password=document.getElementById("password").value;
    firebase.auth().signInWithEmailAndPassword(username, password)
  .then((userCredential) => {
    // Signed in
    var user = userCredential.user;
    username='';
    password='';
   
  })
  .catch((error) => {
    var errorCode = error.code;
    var errorMessage = error.message;
    window.alert(errorMessage)
  });
})

btnLogout.addEventListener('click',function () {
    firebase.auth().signOut().then(() => {
        // Sign-out successful.
      }).catch((error) => {
        // An error happened.
      });
      
    
})