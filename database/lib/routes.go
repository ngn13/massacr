package lib

import (
	"strconv"
	"strings"

	"github.com/gofiber/fiber/v2"
)

var Password string

func Auth(c *fiber.Ctx) error {
  if(c.Query("pass")!=Password){
    return c.Status(403).SendString("Bad password")
  }

  return c.Next()
}

func GETCheck(c *fiber.Ctx) error {
  return c.SendString("All good")
}

func GETSearch(c *fiber.Ctx) error {
  port, err := strconv.Atoi(c.Query("port"))
  if err != nil {
    port = -1
  }
  ip := c.Query("ip")

  results := []Entry{}
  for _, e := range Db.Entries {
    if ip != "" && e.IP != ip {
      continue
    }

    if port > 0 && !CheckPort(e, port){
      continue
    }

    results = append(results, e)
  }

  return c.JSON(results)
}

func GETAdd(c *fiber.Ctx) error {
  rip := c.Query("ip")
  rport, err := strconv.Atoi(c.Query("port"))

  if err != nil || rip == "" || rport <= 0 {
    return c.Status(400).SendString("Bad request")
  }

  for i, e := range Db.Entries {
    if e.IP != rip {
      continue
    }

    if(CheckPort(e, rport)){
      return c.Status(200).SendString("Already exists")
    }

    Db.Entries[i].Ports = append(e.Ports, rport)
    return c.Status(200).SendString("Added port to existing IP")
  }


  Db.Entries = append(Db.Entries, Entry{
    IP: strings.Clone(rip),
    Ports: []int{rport},
  })
  
  return c.Status(200).SendString("Added new IP")
}

