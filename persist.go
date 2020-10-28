package main
import (
	 "sync"
	 "io"
	 "os"
	 "encoding/json"
	 "bytes"
	 "fmt"
	 "log"
)
var lock sync.Mutex

type FileStore struct {
  path string
  lock sync.Mutex
}
// Save saves a representation of v to the file at path.
func (fs FileStore) Save(v interface{}) error {
  fmt.Println("saving")
  fs.lock.Lock()
  defer fs.lock.Unlock()
  f, err := os.Create(fs.path)
  if err != nil {
    fmt.Print(err)
    return err
  }
  defer f.Close()
  r, err := Marshal(v)
  if err != nil {
    fmt.Print(err)
    return err
  }
  _, err = io.Copy(f, r)
  fmt.Print(err)
  return err
}

// Load loads the file at path into v.
// Use os.IsNotExist() to see if the returned error is due
// to the file being missing.
func (fs FileStore) Load(v interface{}) error {
  fmt.Println("loading")
  lock.Lock()
  defer lock.Unlock()
  f, err := os.Open(fs.path)
  if err != nil {
    fmt.Print(err)
    return err
  }
  defer f.Close()
  return Unmarshal(f, v)
}

type Card struct {
  User string
  Rfid string
  Pin  string `json:",omitempty"`
}

type CardStore struct {
  FileStore
  cards map[string]*Card
}

func (cs *CardStore) LoadCards() {
  cards := make(map[string]*Card)
  err := cs.Load(&cards)
  if err != nil {
    log.Print(err.Error())
  }
  cs.cards = cards
}

func (cs *CardStore) DumpCards() {
  err := cs.Save(cs.cards)
  if err != nil {
    log.Print(err.Error())
  }
}


func (cs *CardStore) AddCard(c *Card) {
  cs.cards[c.Rfid] = c
  err := cs.Save(cs.cards)
  if err != nil {
    log.Print(err.Error())
  }
}

func (cs *CardStore) DelCard(rfid string) bool {
  for _, card := range cs.cards {
    if card.Rfid == rfid {
      delete(cs.cards, card.Rfid);
      err := cs.Save(cs.cards)
      if err != nil {
        log.Print(err.Error())
      }
      return true
    }
  }
  return false
}

func (cs *CardStore) GetCards() []Card {
  cards := make([]Card, len(cs.cards))
  pos := int(0)
  for _, card := range cs.cards {
    newCard := Card{User: card.User, Rfid: card.Rfid}
    cards[pos] = newCard
    pos++
  }
  return cards
}

func (cs *CardStore) GetCard(rfid string) (*Card, bool) {
	card, ok := cs.cards[rfid]
	if !ok {
            return &Card{}, ok
        }
	return card, ok
}

// Marshal is a function that marshals the object into an
// io.Reader.
// By default, it uses the JSON marshaller.
var Marshal = func(v interface{}) (io.Reader, error) {
  b, err := json.MarshalIndent(v, "", "\t")
  if err != nil {
    return nil, err
  }
  return bytes.NewReader(b), nil
}

// Unmarshal is a function that unmarshals the data from the
// reader into the specified value.
// By default, it uses the JSON unmarshaller.
var Unmarshal = func(r io.Reader, v interface{}) error {
  return json.NewDecoder(r).Decode(v)
}
