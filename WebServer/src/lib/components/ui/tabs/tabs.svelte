<script lang="ts">
	import { cn, type WithElementRef } from "$lib/utils.js";
	import { setContext } from "svelte";
	import type { HTMLAttributes } from "svelte/elements";
	import { writable } from "svelte/store";
	import { tabsContextKey, type TabsContext } from "./context.js";

	type Props = WithElementRef<HTMLAttributes<HTMLDivElement>, HTMLDivElement> & {
		value?: string;
	};

	let {
		class: className,
		ref = $bindable(null),
		value = $bindable(""),
		children,
		...restProps
	}: Props = $props();

	const selected = writable(value);
	const context: TabsContext = {
		selected,
		setValue: (nextValue) => {
			value = nextValue;
		},
	};

	setContext(tabsContextKey, context);

	$effect(() => {
		selected.set(value);
	});
</script>

<div bind:this={ref} data-slot="tabs" class={cn("flex flex-col gap-4", className)} {...restProps}>
	{@render children?.()}
</div>
