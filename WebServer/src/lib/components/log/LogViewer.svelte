<script lang="ts">
	import type { LogEntry, LogLevel } from "$lib/logs";

	let {
		entries,
		emptyMessage = "No log entries yet."
	}: {
		entries: LogEntry[];
		emptyMessage?: string;
	} = $props();

	let scrollContainer: HTMLDivElement | null = null;
	const entryCount = $derived(entries.length);

	$effect(() => {
		entryCount;
		if (scrollContainer) {
			scrollContainer.scrollTop = scrollContainer.scrollHeight;
		}
	});

	function typeClass(level: LogLevel) {
		switch (level) {
			case "INFO":
				return "text-emerald-700";
			case "WARN":
				return "text-amber-700";
			case "ERR":
				return "text-red-700";
		}
	}
</script>

<div class="flex h-full min-h-72 flex-1 flex-col font-mono text-slate-900">
	<div bind:this={scrollContainer} class="flex min-h-0 flex-1 flex-col overflow-y-auto">
		<div class="mt-auto flex flex-col gap-1.5">
			{#if entries.length === 0}
				<p class="text-sm text-slate-500">{emptyMessage}</p>
			{:else}
				{#each entries as entry}
					<div class="flex flex-wrap items-start gap-x-2 gap-y-1 text-sm leading-6">
						<span class="w-18 shrink-0 text-slate-500">{entry.timestamp}</span>
						<span class={`w-12 shrink-0 font-semibold ${typeClass(entry.type)}`}>{entry.type}</span>
						<span class="min-w-full flex-1 text-slate-700 sm:min-w-0">{entry.message}</span>
					</div>
				{/each}
			{/if}
		</div>
	</div>
</div>
