<script lang="ts">
	import { cn, type WithElementRef } from "$lib/utils.js";
	import { getContext } from "svelte";
	import type { HTMLAttributes } from "svelte/elements";
	import { tabsContextKey, type TabsContext } from "./context.js";

	type Props = WithElementRef<HTMLAttributes<HTMLDivElement>, HTMLDivElement> & {
		value: string;
	};

	let {
		class: className,
		ref = $bindable(null),
		value,
		children,
		...restProps
	}: Props = $props();

	const { selected } = getContext<TabsContext>(tabsContextKey);
</script>

{#if $selected === value}
	<div
		bind:this={ref}
		data-slot="tabs-content"
		class={cn("outline-none", className)}
		{...restProps}
	>
		{@render children?.()}
	</div>
{/if}
