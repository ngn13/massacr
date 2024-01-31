package main

import (
	"log"
	"os"

	"github.com/gofiber/fiber/v2"
	"github.com/gofiber/fiber/v2/middleware/cors"
	"github.com/ngn13/massacr/database/lib"
)

func main(){
  app := fiber.New(fiber.Config{
    DisableStartupMessage: true,
  })

  if(!lib.Load()){
    return
  }

  app.Static("/", "static")
  app.Use(cors.New(cors.Config{
    AllowOrigins: "*",
    AllowHeaders: "*",
    AllowMethods: "GET",
  }))
  app.Use(lib.Auth)
  app.Get("/search", lib.GETSearch)
  app.Get("/check", lib.GETCheck)
  app.Get("/add", lib.GETAdd)

  log.Println("Starting interval based save loop")
  go lib.SaveLoop()

  lib.Password = os.Getenv("PASSWORD")
  if "" == lib.Password {
    lib.Password = "default"
  }

  port := os.Getenv("PORT")
  if "" == port {
    port = ":3231"
  }

  log.Printf("Starting the database on %s", port)
  log.Fatal(app.Listen(port))
}
