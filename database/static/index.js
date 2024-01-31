function getPassword() {
  let password = localStorage.getItem("password")

  if (!password) {
    password = prompt("Enter password:")
    localStorage.setItem("password", password)
  }

  return password
}

function displayResults(data) {
  const resdiv = document.getElementById("results")
  resdiv.innerHTML = ""

  data.forEach(result => {
    const res = document.createElement("div")
    res.classList.add("res")

    const ip = document.createElement("h3")
    ip.textContent = `${result.ip}`
    res.appendChild(ip)

    const ports = document.createElement("div")
    ports.classList.add("ports")

    result.ports.forEach(port => {
      const portel = document.createElement("div")
      portel.classList.add("port")
      portel.textContent = port
      ports.appendChild(portel)
    })

    res.appendChild(ports)
    resdiv.appendChild(res)
  })
}

const form = document.getElementById("form")
form.addEventListener("submit", async(e)=>{
  e.preventDefault()
  const ip = document.getElementById("ip").value
  const port = document.getElementById("port").value
  const password = getPassword()
 
  const res = await fetch(`/search?pass=${password}&ip=${ip}&port=${port}`)
  if (!res.ok) {
    alert("Incorrect password!");
    localStorage.removeItem("password")
  }
 
  const data = await res.json()
  displayResults(data)
})
