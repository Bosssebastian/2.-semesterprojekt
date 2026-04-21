<script lang="ts">
	import { onMount } from "svelte";
	import { LogViewer } from "$lib/components/log";
	import type { LogEntry } from "$lib/logs";
	import { Button } from "$lib/components/ui/button";
	import * as Card from "$lib/components/ui/card";
	import * as Dialog from "$lib/components/ui/dialog";
	import * as Tabs from "$lib/components/ui/tabs";
	import type { ControllerStatus } from "$lib/server/controller";

	type ChartTab = "chart-a" | "chart-b" | "chart-c";
	type ObjectTab = "objects" | "log";
	type PageData = {
		initialStatus: ControllerStatus;
	};

	type CommandName = "start" | "stop";

	let { data } = $props<{ data: PageData }>();

	const chartTabs: Array<{ value: ChartTab; label: string }> = [
		{ value: "chart-a", label: "Chart A" },
		{ value: "chart-b", label: "Chart B" },
		{ value: "chart-c", label: "Chart C" },
	];

	const objectTabs: Array<{ value: ObjectTab; label: string }> = [
		{ value: "objects", label: "Objects" },
		{ value: "log", label: "Log" },
	];

	let activeChartTab = $state<ChartTab>("chart-a");
	let activeObjectTab = $state<ObjectTab>("objects");
	let controllerOnline = $state(false);
	let controllerState = $state("Unavailable");
	let offlineReason = $state<string | null>(null);
	let logEntries = $state<LogEntry[]>([]);
	let logError = $state<string | null>(null);
	let logInterval: number | null = null;

	$effect(() => {
		controllerOnline = data.initialStatus.online;
		controllerState = data.initialStatus.state ?? "Unavailable";
		offlineReason = data.initialStatus.error;
	});

	function isRunningState(state: string) {
		return !["Stopped", "Idle", "Faulted", "Unavailable"].includes(state);
	}

	const stateClasses = $derived.by(() => {
		if (controllerState === "Stopped" || controllerState === "Faulted" || controllerState === "Unavailable") {
			return "border-destructive/20 bg-destructive/10 text-destructive";
		}

		if (isRunningState(controllerState)) {
			return "border-emerald-700/20 bg-emerald-700/12 text-emerald-800";
		}

		return "border-border/80 bg-muted/70 text-foreground";
	});

	async function refreshControllerStatus() {
		const response = await fetch("/api/controller/status");
		const payload = await response.json();

		controllerOnline = payload?.online === true;
		controllerState =
			typeof payload?.state === "string" && payload.state.length > 0 ? payload.state : "Unavailable";
		offlineReason =
			typeof payload?.error === "string" && payload.error.length > 0 ? payload.error : null;
	}

	async function refreshLogs() {
		try {
			const response = await fetch("/api/controller/logs");
			const payload = await response.json();

			if (!response.ok) {
				logEntries = [];
				logError =
					typeof payload?.error === "string" ? payload.error : "Failed to fetch controller logs";
				return;
			}

			logEntries = Array.isArray(payload?.entries) ? payload.entries : [];
			logError = null;
		} catch (error) {
			logEntries = [];
			logError = error instanceof Error ? error.message : "Failed to fetch controller logs";
		}
	}

	function startLogPolling() {
		if (typeof window === "undefined" || logInterval !== null) {
			return;
		}

		void refreshLogs();
		logInterval = window.setInterval(() => {
			void refreshLogs();
		}, 1000);
	}

	function stopLogPolling() {
		if (typeof window === "undefined" || logInterval === null) {
			return;
		}

		window.clearInterval(logInterval);
		logInterval = null;
	}

	async function sendCommand(command: CommandName) {
		if (!controllerOnline) {
			return;
		}

		try {
			const response = await fetch(`/api/controller/${command}`, {
				method: "POST"
			});
			const payload = await response.json();

			if (!response.ok) {
				throw new Error(
					typeof payload?.error === "string" ? payload.error : "Failed to fetch controller state"
				);
			}

			await refreshControllerStatus();
		} catch (error) {
			await refreshControllerStatus();
		}
	}

	onMount(() => {
		const interval = window.setInterval(() => {
			void refreshControllerStatus();
		}, 1000);

		void refreshControllerStatus();

		return () => {
			stopLogPolling();
			window.clearInterval(interval);
		};
	});

	$effect(() => {
		if (activeObjectTab === "log") {
			startLogPolling();
			return;
		}

		stopLogPolling();
	});
</script>

<svelte:head>
	<title>Robotics Controller Dashboard</title>
	<meta
		name="description"
		content="Mockup dashboard for controlling a C++ robotics controller."
	/>
</svelte:head>

<Dialog.Root open={!controllerOnline}>
	<Dialog.Content>
		<Dialog.Header>
			<p class="text-xs font-semibold uppercase tracking-[0.28em] text-destructive">Controller Offline</p>
			<Dialog.Title>The C++ controller is not reachable.</Dialog.Title>
			<Dialog.Description>
				Start the PC controller process and keep it listening on `127.0.0.1:8081`, then retry the connection.
			</Dialog.Description>
			{#if offlineReason}
				<div class="rounded-[1.1rem] border border-destructive/15 bg-destructive/8 px-4 py-3 text-sm text-destructive">
					{offlineReason}
				</div>
			{/if}
		</Dialog.Header>

		<Dialog.Footer>
			<Button
				onclick={() => void refreshControllerStatus()}
				class="rounded-[1.2rem] bg-slate-900 px-5 py-3 text-sm font-semibold text-white hover:bg-slate-800"
			>
				Retry Connection
			</Button>
		</Dialog.Footer>
	</Dialog.Content>
</Dialog.Root>

<section class="h-screen overflow-hidden p-4 sm:p-6 lg:p-8">
	<div class="flex h-full w-full flex-col">
		<div class="grid min-h-0 flex-1 gap-6 lg:grid-cols-[minmax(0,1.05fr)_minmax(0,0.95fr)]">
			<div class="grid min-h-0 gap-6 lg:grid-rows-[minmax(0,1fr)_auto]">
				<Tabs.Root bind:value={activeChartTab} class="min-h-0 gap-0">
					<div class="px-4 sm:px-6">
						<Tabs.List class="-mb-5">
							{#each chartTabs as tab}
								<Tabs.Trigger value={tab.value}>{tab.label}</Tabs.Trigger>
							{/each}
						</Tabs.List>
					</div>

					<Card.Root class="flex min-h-0 flex-1 flex-col overflow-hidden border-slate-300/70 bg-white/92">
						<Card.Content class="flex min-h-0 flex-1 pt-8">
							{#each chartTabs as tab}
								<Tabs.Content value={tab.value} class="flex min-h-0 flex-1">
									<div class="min-h-72 flex-1 lg:min-h-0"></div>
								</Tabs.Content>
							{/each}
						</Card.Content>
					</Card.Root>
				</Tabs.Root>

				<Card.Root class="min-h-72 shrink-0 border-slate-300/70 bg-white/95 shadow-[0_20px_50px_-30px_rgba(15,23,42,0.55)] lg:min-h-[20rem]">
					<Card.Content class="h-full pt-6">
						<div class="flex h-full flex-col gap-6">
							<div class="flex flex-col gap-4">
								<div class="flex w-full items-center gap-4 ps-4">
									<span class="shrink-0 text-base font-semibold uppercase tracking-[0.18em] text-muted-foreground">
										State
									</span>
									<div
										class={`min-w-0 flex-1 rounded-[1.2rem] border px-5 py-4 text-center font-semibold uppercase tracking-[0.24em] shadow-inner ${stateClasses}`}
									>
										{controllerState}
									</div>
								</div>
							</div>

							<div class="grid flex-1 gap-4 sm:grid-cols-2">
								<Button
									onclick={() => void sendCommand("start")}
									size="lg"
									class="h-24 rounded-[1.9rem] border border-emerald-700/20 bg-emerald-700 text-2xl font-semibold uppercase tracking-[0.16em] text-white shadow-[0_18px_36px_-22px_rgba(4,120,87,0.9)] hover:bg-emerald-600 sm:h-28 lg:h-full lg:min-h-[8.75rem]"
								>
									Start
								</Button>
								<Button
									onclick={() => void sendCommand("stop")}
									variant="destructive"
									size="lg"
									class="h-24 rounded-[1.9rem] border border-destructive/20 bg-destructive text-2xl font-semibold uppercase tracking-[0.16em] text-white shadow-[0_18px_36px_-22px_rgba(185,28,28,0.85)] hover:bg-red-600 sm:h-28 lg:h-full lg:min-h-[8.75rem]"
								>
									Stop
								</Button>
							</div>
						</div>
					</Card.Content>
				</Card.Root>
			</div>

			<Tabs.Root bind:value={activeObjectTab} class="min-h-0 gap-0">
				<div class="px-4 sm:px-6">
					<Tabs.List class="-mb-5">
						{#each objectTabs as tab}
							<Tabs.Trigger value={tab.value}>{tab.label}</Tabs.Trigger>
						{/each}
					</Tabs.List>
				</div>

				<Card.Root
					class={`flex min-h-0 flex-1 flex-col overflow-hidden border-slate-300/70 ${
						activeObjectTab === "log" ? "bg-slate-50 text-slate-900" : "bg-white/92"
					}`}
				>
					<Card.Content
						class={`flex min-h-0 flex-1 ${activeObjectTab === "log" ? "p-6 pt-6" : "pt-8"}`}
					>
						{#each objectTabs as tab}
							<Tabs.Content value={tab.value} class="flex min-h-0 flex-1">
								{#if tab.value === "log"}
									<LogViewer entries={logEntries} emptyMessage={logError ?? "No log entries yet."} />
								{:else}
									<div class="min-h-72 flex-1 lg:min-h-0"></div>
								{/if}
							</Tabs.Content>
						{/each}
					</Card.Content>
				</Card.Root>
			</Tabs.Root>
		</div>
	</div>
</section>
