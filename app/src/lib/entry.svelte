<script>
  export let ipv4;
  export let ports;
</script>

<main>
  <h1>{ipv4}</h1>
  <div class="ports">
  {#each ports as p}
    <p>{p}</p>
  {/each}
  </div>
</main>

<style>
main {
  border-radius: var(--radius);
  border: solid 2px var(--red-1);
  padding: 15px;
}

main h1 {
  color: var(--red-1);
  margin-bottom: 7px;
}

main .ports {
  display: flex;
  flex-direction: row;
  align-items: center;
  flex-wrap: wrap;
  gap: 10px;
}

main .ports p {
  color: var(--bright-1);
  font-size: 15px;
  background: var(--red-2);
  padding: 5px 8px;
  border-radius: var(--radius);
  font-weight: 500;
}
</style>
