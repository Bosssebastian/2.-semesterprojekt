<script lang="ts">
	import { cn, type WithElementRef } from "$lib/utils.js";
	import type { HTMLAttributes } from "svelte/elements";

	type Props = WithElementRef<HTMLAttributes<HTMLDivElement>, HTMLDivElement>;

	let { class: className, ref = $bindable(null), children, ...restProps }: Props = $props();
</script>

<div
	bind:this={ref}
	data-slot="card"
	class={cn(
		"bg-card text-card-foreground flex flex-col rounded-[2rem] border border-border/80 shadow-[0_24px_60px_-32px_rgba(15,23,42,0.45)]",
		className
	)}
	{...restProps}
>
	{@render children?.()}
</div>
