<script lang="ts">
	import { cn, type WithElementRef } from "$lib/utils.js";
	import type { HTMLAttributes } from "svelte/elements";

	type Props = WithElementRef<HTMLAttributes<HTMLDivElement>, HTMLDivElement>;

	let { class: className, ref = $bindable(null), children, ...restProps }: Props = $props();
</script>

<div
	bind:this={ref}
	data-slot="card-content"
	class={cn("px-6 pb-6", className)}
	{...restProps}
>
	{@render children?.()}
</div>
