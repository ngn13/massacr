<script>
  import Entry from "../lib/entry.svelte";
  import { goto } from '$app/navigation';
  import { page } from '$app/stores';

  export let data;
  let query, current = data.page;

  async function go() {
    console.log(current.toString().length)
    current = parseInt(current)
    if(isNaN(current) || current <= 0){
      current = 1;
      return;
    }

    let url = new URLSearchParams($page.url.searchParams.toString());
    url.set("page", current);
    goto(`?${url.toString()}`);
  }

  async function search(e) {
    if(e.key != "Enter")
      return;

    current = 1;
    let url = new URLSearchParams($page.url.searchParams.toString());
    url.set("page", current);
    url.set("query", query);
    goto(`?${url.toString()}`);
  }

  async function next() {
    if(data.page >= data.pages)
      return;

    current++;
    await go();
  }

  async function prev() {
    if(data.page <= 1)
      return;

    current--;
    await go();
  }

  function get_len(n) {
    let len = n.toString().length;
    len = len===0 ? 1 : len;
    return len;
  }
</script>

<main>
  <div class="info">
    <h1>./massacr</h1>
    <div class="page">
      <h3>Total of <p class="red">{data.size.toLocaleString()}</p> results</h3>
      <h3>Total of <p class="red">{data.pages.toLocaleString()}</p> pages</h3>
      <div class="buttons">
        {#if data.page != 1}
          <button class="active" on:click={async()=>{await prev()}}><i class="nf nf-md-skip_previous"></i></button>
        {:else}
          <button class="deactive"><i class="nf nf-md-skip_previous"></i></button>
        {/if}
        <h3><input on:change={async()=>{await go()}} type="text" size="{get_len(current)}" bind:value="{current}" placeholder="1">/{data.pages}</h3>
        {#if data.page < data.pages}
          <button class="active" on:click={async()=>{await next()}}><i class="nf nf-md-skip_next"></i></button>
        {:else}
          <button class="deactive"><i class="nf nf-md-skip_next"></i></button>
        {/if}
      </div>
    </div>
  </div>
  <div class="search">
    <div class="bar">
      <p><i class="nf nf-seti-search"></i></p>
      <input bind:value={query} on:keydown={async(e)=>{await search(e)}} type="text" placeholder="ipv4:1.2.3.4 port:80 port:443" />
    </div>
    {#if data.error}
      <h3 class="red error">{data.error}</h3>
    {:else if data.size != 0}
      <div class="results">
        {#each data.list as e}
          <Entry ipv4={e.ipv4} ports={e.ports}></Entry>
        {/each}
      </div>
    {/if}
  </div>
</main>

<style>
  main {
    display: flex;
    gap: 30px;
    position: absolute;
    top: 50%;
    left: 50%;
    transform: translate(-50%, -50%);
  }

  .info {
    display: flex;
    flex-direction: column;
    gap: 15px;
    padding-right: 30px;
    border-right: solid 2px var(--red-2);
  }

  .info h1 {
    color: var(--red-1);
    text-shadow:
      20px 7px 5px rgba(80, 0, 0, 0.3),
      20px -4px 10px rgba(255, 0, 0, 0.3);
    font-size: 30px;
  }

  .red {
    color: var(--red-1);
  }

  .error {
    text-align: center;
    font-size: 20px;
    padding: 30px;
  }

  .page {
    background: var(--dark-1);
    border-radius: var(--radius);
    border: solid 1px var(--red-1);
    padding: 20px;
    height: min-content;
  }

  .page h3 {
    display: flex;
    flex-direction: row;
    align-items: center;
    color: var(--bright-1);
    font-size: 20px;
    gap: 10px;
  }

  .buttons h3 {
    display: flex;
    flex-direction: row;
    align-items: center;
    gap: 1px;
  }

  .buttons h3 input {
    resize: horizontal;
    border: none;
    background: none;
    color: var(--bright-1);
    font-weight: 900;
    text-align: right;
    font-size: 20px;
    outline: none;
  }
  
  .buttons h3 input:focus {
    border: solid 1px var(--red-1);
  }

  .page .buttons {
    display: flex;
    flex-direction: row;
    justify-content: space-between;
    align-items: center;
    padding-top: 10px;
    gap: 20px;
  }

  .page .buttons button {
    border: none;
    outline: none;
    padding: 8px 15px;
    font-size: 10px;
    border-radius: var(--radius);
  }

  .active {
    background: var(--red-1);
    color: var(--bright-1);
    cursor: pointer;
  }

  .deactive {
    background: var(--red-2);
    color: var(--bright-2);
    cursor: default;
  }

  .search {
    width: max-content;
    display: flex;
    flex-direction: column;
    width: 400px;
    gap: 15px;

    height: 500px;
    overflow: -moz-scrollbars-vertical;
    overflow-x: hidden;
    overflow-y: scroll;
    padding-right: 20px;
  }

  .bar {
    display: flex;
    flex-direction: row;
    border: solid 1px var(--red-1);
    border-radius: var(--radius);
    padding: 10px 12px;
    gap: 10px;
    box-shadow: rgba(250, 20, 20, 0.25) 0px 30px 60px -12px inset, rgba(0, 0, 0, 0.3) 0px 18px 36px -18px inset;
  }

  .bar input {
    color: var(--bright-1);
    font-size: 18px;
    text-align: right;

    background: none;
    outline: none;
    border: none;
    
    width: 100%;
  }

  .bar p {
    color: var(--red-1);
    font-size: 18px;
  }

  .results {
    display: flex;
    flex-direction: column;
    gap: 15px;
  }
</style>
