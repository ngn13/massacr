package lib 

import (
	"encoding/json"
	"log"
	"os"
	"strconv"
	"time"
)

type Entry struct {
  IP    string  `json:"ip"`
  Ports []int   `json:"ports"`
}

type Data struct {
  Entries []Entry   `json:"entries"`
}

var Db Data
var save_tick = time.NewTicker(Interval())

func SaveLoop() {
  for {
    select {
      case <- save_tick.C:
        Save()
    }
  }
}

func Interval() time.Duration {
  secs, err := strconv.Atoi(os.Getenv("INTERVAL"))
  if err != nil || secs <= 0 {
    return 30*time.Second
  }

  return time.Duration(secs)*time.Second
}

func Save() bool{
  dbstr, err := json.MarshalIndent(Db, "", "  ")
  if err != nil {
    log.Printf("Failed to save database: %s", err)
    return false
  }

  err = os.Mkdir("data", os.ModePerm)
  if err != nil && !os.IsExist(err) {
    log.Printf("Cannot create the data directory")
    return false
  }

  err = os.WriteFile("data/data.json", dbstr, 0666)
  if err != nil {
    log.Printf("Failed to save database: %s", err)
    return false
  }

  return true
}

func Load() bool {
  dbstr, err := os.ReadFile("data/data.json")
  if os.IsNotExist(err) {
    Db = Data{}
    return true 
  }

  err = json.Unmarshal(dbstr, &Db)
  if err != nil {
    log.Printf("Failed to load database: %s", err)
    return false
  }

  return true
}

func CheckPort(en Entry, port int) bool{
  for _, p := range en.Ports {
    if port == p {
      return true
    }
  }

  return false
}
