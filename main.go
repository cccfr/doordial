package main

import (
	//"crypto/tls"
    "crypto/rand"
    "fmt"
	"net/http"

	//"golang.org/x/crypto/acme/autocert"
    "strings"
    "strconv"
    "log"
    "io"
    "io/ioutil"
    "os/exec"
    "encoding/json"
)

var openChallenges map[string]string
var userDB map[string]string
var table = [...]byte{'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'}
var unknownCards []string

var cardStore CardStore

func main() {
    openChallenges = make(map[string]string)
    unknownCards = make([]string, 5)
    cardStore = CardStore{FileStore: FileStore{path: "/opt/doordial/cards"}, cards: make(map[string]*Card)}
    cardStore.LoadCards()
    //userDB = make(map[string]string)
    //userDB["3841001278"] = "0000"
    cardStore.AddCard(&Card{User: "test", Rfid: "3841001278", Pin: "0000"})
    fmt.Printf("starting with %v users\n", len(cardStore.cards))

    mux := http.NewServeMux()
    mux.HandleFunc("/api/startchallenge", handleChallengeRequest)
    mux.HandleFunc("/api/solvechallenge", handleChallengeResponse)
    mux.HandleFunc("/api/newcard", handleNewCard)
    mux.HandleFunc("/api/delcard", handleDelCard)
    mux.HandleFunc("/api/cards", handleGetCards)
    mux.HandleFunc("/api/unknowns", handleGetUnknowns)

    /*
	certManager := autocert.Manager{
		Prompt: autocert.AcceptTOS,
		Cache:  autocert.DirCache("certs"),
	}

    server := &http.Server{
		Addr:    ":8443",
		Handler: mux,
		TLSConfig: &tls.Config{
			GetCertificate: certManager.GetCertificate,
		},
	}
    */


    //err := http.ListenAndServeTLS(":8443", "cert.pem", "key.pem", mux)
    fmt.Println("listen on *:8080")
    err := http.ListenAndServe(":8080", mux)
    if err != nil {
	    log.Printf("connot listen: %v", err.Error())
            return
    }
    //fmt.Println("starting http")
	//go http.ListenAndServe(":8080", certManager.HTTPHandler(nil))
    //fmt.Println("starting https")
    //err := server.ListenAndServeTLS("", "")
    //fmt.Println(err)
}

func handleDebug(w http.ResponseWriter, r *http.Request) {
    log.Println("DEBUG")
}

func open() {
        log.Println("Opening...")
	return
        cmd := exec.Command("/opt/sesam")
	result, err := cmd.Output()
        if err != nil {
            log.Println(err.Error())
        }
        log.Println(string(result))
}

// CHALLENGE HANDLING

func handleChallengeRequest(w http.ResponseWriter, r *http.Request) {
    log.Println("handleChallengeRequest")
    if r.Method != "POST" {
        log.Printf("wrong method: %v", r.Method)
        http.Error(w, "OhWm wrong method", http.StatusBadRequest)
        return
    }
    body, err := ioutil.ReadAll(r.Body)
    if err != nil {
        log.Printf("Error reading body: %v", err)
        http.Error(w, "OhRb can't read body", http.StatusBadRequest)
        return
    }
    rfid_raw := strings.Split(string(body), "l")
    rfid := rfid_raw[0]
    card, ok := cardStore.GetCard(rfid)
    pin := card.Pin
    if !ok {
        log.Printf("rfid \"%s\" unknown", rfid)
	if len(unknownCards) >= 5 {
		unknownCards = unknownCards[len(unknownCards)-4:]
	}
	unknownCards = append(unknownCards, rfid)
        http.Error(w, "OhNo! rfid unknown", http.StatusBadRequest)
        return
    }
    log.Printf("request from %s\n", rfid)
    openChallenges[rfid] = generateChallenge(pin)
    fmt.Fprintf(w, "OkKc", openChallenges[rfid])
    fmt.Printf("sent challenge %s\n", openChallenges[rfid])
}

func handleChallengeResponse(w http.ResponseWriter, r *http.Request) {
    log.Println("handleChallengeResponse")
    if r.Method != "POST" {
        log.Printf("wrong method: %v", r.Method)
        http.Error(w, "OhWm wrong method", http.StatusBadRequest)
        return
    }
    body, err := ioutil.ReadAll(r.Body)
    if err != nil {
        log.Printf("Error reading body: %v", err)
        http.Error(w, "OhRb can't read body", http.StatusBadRequest)
        return
    }
    log.Print("incoming challenge")
    log.Print(string(body))
    result := strings.Split(string(body), "#")
    if len(result) != 2 {
        log.Printf("wrong input: %v", result)
        http.Error(w, "OhWi wrong input", http.StatusBadRequest)
        return
    }
    rfid := strings.Trim(result[0], "l")
    input := result[1]
    challenge, ok := openChallenges[rfid]
    if !ok {
        log.Print("challenge unknown")
        http.Error(w, "OhCuchallenge unknown", http.StatusBadRequest)
        return
    }
    log.Print("input: ")
    log.Println(input)
    //fmt.Printf("input: %s, challenge: %s\n", input, challenge)
    pin := solveChallenge(input, challenge)
    card, ok := cardStore.GetCard(rfid)
    if card.Pin == "0000" || card.Pin == ""{
        card.Pin = pin
        log.Printf("set pin for %s", rfid)
	cardStore.DumpCards();
        return
    }
    if pin != card.Pin {
        log.Printf("wrong pin \"%s\"", pin)
        http.Error(w, "OhWp! wrong pin", http.StatusBadRequest)
        return
    }
    log.Printf("Access Granted to %s\n", rfid)
    open()
    fmt.Fprintf(w, "OkAg Access Granted")
}

// WEBFRONTEND
func handleNewCard(w http.ResponseWriter, r *http.Request) {
    if r.Method != "POST" {
        log.Printf("wrong method: %v", r.Method)
        http.Error(w, "wrong method", http.StatusBadRequest)
        return
    }
    body, err := ioutil.ReadAll(r.Body)
    if err != nil {
        log.Printf("Error reading body: %v", err)
        http.Error(w, "can't read body", http.StatusBadRequest)
        return
    }
    log.Print("incoming newCard")
    bodycard := Card{}
    fmt.Println(string(body))
    err = json.Unmarshal(body, &bodycard)
    if err != nil {
        log.Printf("Error unmarshaling body: %v", err)
        http.Error(w, "can't unmarshal body", http.StatusBadRequest)
        return
    }
    log.Printf("%v+", bodycard)
    cardStore.AddCard(&Card{User: bodycard.User, Rfid: bodycard.Rfid, Pin: "0000"})
    cardStore.LoadCards()
    log.Println(" saved")
}

func handleDelCard(w http.ResponseWriter, r *http.Request) {
    if r.Method != "POST" {
        log.Printf("wrong method: %v", r.Method)
        http.Error(w, "wrong method", http.StatusBadRequest)
        return
    }
    body, err := ioutil.ReadAll(r.Body)
    if err != nil {
        log.Printf("Error reading body: %v", err)
        http.Error(w, "can't read body", http.StatusBadRequest)
        return
    }
    log.Print("incoming delCard")
    bodycard := Card{}
    fmt.Println(string(body))
    err = json.Unmarshal(body, &bodycard)
    if err != nil {
        log.Printf("Error unmarshaling body: %v", err)
        http.Error(w, "can't unmarshal body", http.StatusBadRequest)
        return
    }
    log.Printf("%v+", bodycard)
    ok := cardStore.DelCard(bodycard.Rfid)
    if !ok {
        log.Print("cannot delete card")
	http.Error(w, "cannot delete Card", 501)
	return
    }
    cardStore.LoadCards()
    log.Println(" saved")
}

func handleGetCards(w http.ResponseWriter, r *http.Request) {
    log.Println("giving out cards")
    cards := cardStore.GetCards()
    fmt.Println(cards)
    output, err := json.Marshal(cards)
    if err != nil {
        return
    }
    fmt.Fprint(w, string(output))
}

func handleGetUnknowns(w http.ResponseWriter, r *http.Request) {
    log.Println("giving out unknowns")
    fmt.Println(unknownCards)
    output, err := json.Marshal(unknownCards)
    if err != nil {
        return
    }
    fmt.Fprint(w, string(output))
}

func generateChallenge(pin string) string {
    b := make([]byte, 4)
    n, err := io.ReadAtLeast(rand.Reader, b, 4)
    if n != 4 {
        panic(err)
    }
    for i := 0; i < len(b); i++ {
        b[i] = table[(int(b[i])+int(pin[i]))%len(table)]
    }
    return string(b)
}

func solveChallenge(input, challenge string) string {
    pin := make([]byte, 4)
    for i := 0; i < 4; i++ {
        inDigit, _:= strconv.Atoi(string(input[i]))
        challDigit, _ := strconv.Atoi(string(challenge[i]))
        //fmt.Printf("%i - %i = ", challDigit, inDigit)
        tmp := challDigit-inDigit
        if tmp < 0 {
            tmp = 10 + tmp
        }
        //fmt.Println(tmp)
        pin[i] = table[tmp]
    }
    //fmt.Println(string(pin))
    return string(pin)
}
