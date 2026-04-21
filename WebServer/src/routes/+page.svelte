<script lang="ts">
	import { Button } from "$lib/components/ui/button";
	import * as Card from "$lib/components/ui/card";
	import * as Tabs from "$lib/components/ui/tabs";

	type ChartTab = "chart-a" | "chart-b" | "chart-c";
	type ObjectTab = "objects" | "groups";
	type ControllerState = "Idle" | "Running" | "Stopped";

	const chartTabs: Array<{ value: ChartTab; label: string }> = [
		{ value: "chart-a", label: "Chart A" },
		{ value: "chart-b", label: "Chart B" },
		{ value: "chart-c", label: "Chart C" },
	];

	const objectTabs: Array<{ value: ObjectTab; label: string }> = [
		{ value: "objects", label: "Objects" },
		{ value: "groups", label: "Groups" },
	];

	let activeChartTab = $state<ChartTab>("chart-a");
	let activeObjectTab = $state<ObjectTab>("objects");
	let controllerState = $state<ControllerState>("Idle");

	const stateVariant = $derived.by(() => {
		if (controllerState === "Running") return "success";
		if (controllerState === "Stopped") return "destructive";
		return "secondary";
	});
</script>

<svelte:head>
	<title>Robotics Controller Dashboard</title>
	<meta
		name="description"
		content="Mockup dashboard for controlling a C++ robotics controller."
	/>
</svelte:head>

<section class="h-screen overflow-hidden bg-[radial-gradient(circle_at_top_left,_rgba(255,255,255,0.95),_rgba(245,245,245,0.98)_35%,_rgba(229,231,235,0.96)_100%)] p-4 sm:p-6 lg:p-8">
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
									<div class="min-h-[18rem] flex-1 rounded-[1.75rem] border border-border/80 bg-[linear-gradient(180deg,rgba(255,255,255,0.96),rgba(248,250,252,0.88))] lg:min-h-0"></div>
								</Tabs.Content>
							{/each}
						</Card.Content>
					</Card.Root>
				</Tabs.Root>

				<Card.Root class="min-h-[18rem] shrink-0 border-slate-300/70 bg-white/95 shadow-[0_20px_50px_-30px_rgba(15,23,42,0.55)] lg:min-h-[20rem]">
					<Card.Content class="h-full pt-6">
						<div class="flex h-full flex-col gap-6">
							<div class="flex flex-col gap-4">

								<div class="flex w-full items-center gap-4 rounded-[1.6rem] border border-border/80 bg-background/95 px-5 py-4 shadow-inner">
									<span class="shrink-0 text-base font-semibold uppercase tracking-[0.18em] text-muted-foreground">
										State
									</span>
									<div
										class={`min-w-0 flex-1 rounded-[1.2rem] border px-5 py-4 text-center text-xl font-semibold uppercase tracking-[0.24em] shadow-inner lg:text-2xl ${
											controllerState === "Running"
												? "border-emerald-700/20 bg-emerald-700/12 text-emerald-800"
												: controllerState === "Stopped"
													? "border-destructive/20 bg-destructive/10 text-destructive"
													: "border-border/80 bg-muted/70 text-foreground"
										}`}
									>
										{controllerState}
									</div>
								</div>
							</div>

							<div class="grid flex-1 gap-4 sm:grid-cols-2">
								<Button
									size="lg"
									class="h-24 rounded-[1.9rem] border border-emerald-700/20 bg-emerald-700 text-2xl font-semibold uppercase tracking-[0.16em] text-white shadow-[0_18px_36px_-22px_rgba(4,120,87,0.9)] hover:bg-emerald-600 sm:h-28 lg:h-full lg:min-h-[8.75rem]"
								>
									Start
								</Button>
								<Button
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

				<Card.Root class="flex min-h-0 flex-1 flex-col overflow-hidden border-slate-300/70 bg-white/92">
					<Card.Content class="flex min-h-0 flex-1 pt-8">
						{#each objectTabs as tab}
							<Tabs.Content value={tab.value} class="flex min-h-0 flex-1">
								<div class="min-h-[18rem] flex-1 rounded-[1.9rem] border border-border/80 bg-[linear-gradient(180deg,rgba(255,255,255,0.98),rgba(241,245,249,0.75))] lg:min-h-0"></div>
							</Tabs.Content>
						{/each}
					</Card.Content>
				</Card.Root>
			</Tabs.Root>
		</div>
	</div>
</section>
