import * as functions from 'firebase-functions';
import * as admin from 'firebase-admin';

admin.initializeApp(functions.config().firebase);

export const getSettings = functions.https.onRequest((request, response) => {
    const db = admin.firestore();
    let settings;
    db.collection("settings")
        .doc("currentSettings")
        .get()
        .then(docSnapshot => {
            settings = docSnapshot.data();
            console.log(settings);
            response.send(settings);
        }).catch(error => {
            console.error(error);
            response.send("An error occured while waching the database");
        });
});

export const setWeather = functions.https.onRequest((request, response) => {
    const db = admin.firestore();
    db.collection("settings").doc("currentSettings")
        .update({ weather: request.body })
        .then(docSnapshot => {
            console.log("Successfuly updated to", request.body);
            response.sendStatus(201);        
        }).catch(error => {
            console.error(error);
            response.sendStatus(500);
        })
    
});

export const motionDetected = functions.https.onRequest((request, response) => {
    let state= false;
    let peopleInside;
    if (request.body == "1") {
        state = true;
    } else {
        state = false;
    }

    const db = admin.firestore();

    const docRef = db.doc("settings/currentSettings");
    db.runTransaction(t => {
        return t.get(docRef)
            .then(doc => {
                let settings = doc.data();
                if (settings) {
                    if (state) {
                        if (settings.peopleInside == 0) {
                            t.update(docRef, { lighting: 100 });
                        }
                        peopleInside = settings.peopleInside + 1;
                    } else {
                        peopleInside = settings.peopleInside - 1;
                        if (peopleInside == 0) {
                            t.update(docRef, { lighting: 0 });
                        }
                    }
                    t.update(docRef, { peopleInside });
                }
            }).catch(error => console.error(error));
    }).then(result => {
        console.log("transaction completed");
        response.send(200);
    }).catch(error => console.error(error));
});

export const switchLights = functions.https.onRequest((request, response) => {
    const db = admin.firestore();
    const docRef = db.doc("settings/currentSettings");
    const state = request.body;
    // console.log("body: ", state);
    docRef
        .update({ lighting: state == "0" ? 0 : 100 })
        .then(() => {
            console.log("Updated lighting", "state", state);
            response.send(200);
        }).catch(error => console.error("couldn't update lighitng\n", error));
    
    
});

export const watchEdits = functions.firestore
    .document("settings/currentSettings")
    .onUpdate((snap, context) => {
        console.log("Someone used the app!");
    });